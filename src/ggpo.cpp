#include "ggpo.h"

#include "game_state_debug.h"
#include "memory_dump.h"
#include "util.h"

#include <ggponet.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <queue>
#include <unordered_map>
#include <unordered_set>

#include <Windows.h>
#include <Winsock2.h>


#define GGPO_CHECK(expr) \
    do { \
        const auto res___ = expr; \
        if (!GGPO_SUCCEEDED(res___)) { \
            std::cerr << __FILE__ << ':' << __LINE__ << ": " << #expr << " == " << res___ << std::endl; \
            throw std::logic_error("GGPO error"); \
        } \
    } while(false)

namespace ggpo
{

namespace
{

GGPOSession* g_session = nullptr;
GGPOPlayerHandle g_player_handles[2];
size_t g_frame_base = 0;
std::unordered_map<size_t, std::shared_ptr<game_state>> g_saved_state_map;
size_t g_vs_2p_jmp_addr = 0;
IGame* g_game = nullptr;
configuration* g_cfg = nullptr;
bool g_manual_frame_advance_enabled_backup = false;
bool g_network_enabled = false;
bool g_disconnected = false;
std::string g_status_msg = "";
uint32_t g_timesync_frames = 0;
bool g_status_displayed = false;
bool g_ggpo_synchronized = false;


#pragma warning(push)
// warning: flow in or out of inline asm code suppresses global optimization
#pragma warning(disable: 4740)
void __declspec(naked) jmp_menu_network()
{
    __asm {
        push eax
        push ebx
        push ecx
        push edx
        push esi
        push edi
    }
    g_network_enabled = true;
    __asm {
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        jmp g_vs_2p_jmp_addr
    }
}
#pragma warning(pop)

using memory_dump::dump;
using memory_dump::load;

void apply_patches(size_t image_base)
{
    load(image_base + 0x226448 + 2 * 4, g_vs_2p_jmp_addr);
    // Replace main menu jump table entry for NETWORK
    void* ptr = jmp_menu_network;
    dump(ptr, image_base + 0x226448 + 3 * 4);
}

namespace ggpo_callbacks
{

bool begin_game(const char*)
{
    return true;
}

bool save_game_state(unsigned char **buffer, int *len, int *checksum, int frame)
{
    auto state_ptr = std::make_shared<game_state>(g_game->GetState());
    LIBGG_LOG() << state_ptr->match.frame.get() << std::endl;
    assert(state_ptr->match.frame.get() - g_frame_base == static_cast<size_t>(frame));

    *buffer = (unsigned char*)state_ptr.get();
    *len = sizeof(game_state);

    if (g_cfg->get_args().synctest_frames > 0)
    {
        // strict checksum: checksum that involves noninteractive objects and pointers
        bool strict = g_cfg->get_args().synctest_strict;
        *checksum = state_checksum(*state_ptr, strict);
        auto found = g_saved_state_map.find(frame);
        if (found != g_saved_state_map.end())
        {
            int checksum_old = state_checksum(*found->second, strict);
            if (checksum_old != *checksum)
            {
                std::cout << "Synctest failed" << std::endl;
                print_game_state(*found->second);
                print_game_state(*state_ptr);
                std::exit(1);
            }
        }
    }
    g_saved_state_map[frame] = state_ptr;

    return true;
}

bool load_game_state(unsigned char *buffer, int /* len */)
{
    auto state_ptr = (game_state*)(buffer);
    LIBGG_LOG() << state_ptr->match.frame.get() << std::endl;
    g_game->SetState(*state_ptr);
    return true;
}

bool log_game_state(char* /* filename */, unsigned char* /* buffer */, int /* len */)
{
    return true;
}

void free_buffer(void *buffer)
{
    if (!buffer)
        return;

    auto state = (game_state*)buffer;
    auto frame = state->match.frame.get() - g_frame_base;
    LIBGG_LOG() << frame << std::endl;
    if (g_cfg->get_args().synctest_frames == 0)
    {
        auto removed = g_saved_state_map.erase(frame);
        (void)removed;
        assert(removed == 1);
    }
    else
    {
        static std::queue<uint32_t> s_erase_queue;
        s_erase_queue.push(frame);
        while (s_erase_queue.size() > 100)
        {
            auto frame = s_erase_queue.front();
            g_saved_state_map.erase(frame);
            s_erase_queue.pop();
        }
    }
}

bool advance_frame(int)
{
    LIBGG_LOG() << std::endl;
    int disconnected = 0;
    IGame::input_t input{};
    GGPO_CHECK(ggpo_synchronize_input(g_session, (void*)&input, sizeof(input), &disconnected));
    if (disconnected)
        g_disconnected = true;
    g_game->SetCachedInput(g_game->RemapInputFromDefault(input));
    g_game->AdvanceFrame();
    g_game->ProcessAudio();
    return true;
}

bool on_event(GGPOEvent *info)
{
    LIBGG_LOG() << info->code << std::endl;
    switch (info->code)
    {
    case GGPO_EVENTCODE_CONNECTED_TO_PEER:
        g_status_msg = "CONNECTED";
        break;
    case GGPO_EVENTCODE_SYNCHRONIZING_WITH_PEER:
        g_status_msg = "SYNCHRONIZING " + std::to_string(info->u.synchronizing.count) + '/' + std::to_string(info->u.synchronizing.total);
        break;
    case GGPO_EVENTCODE_SYNCHRONIZED_WITH_PEER:
        g_status_msg = "SYNCHRONIZED";
        g_ggpo_synchronized = true;
        break;
    case GGPO_EVENTCODE_RUNNING:
        g_status_msg = "";
        break;
    case GGPO_EVENTCODE_CONNECTION_INTERRUPTED:
        g_status_msg = "CONNECTION INTERRUPTED";
        g_ggpo_synchronized = false;
        break;
    case GGPO_EVENTCODE_CONNECTION_RESUMED:
        g_status_msg = "CONNECTION RESUMED";
        g_ggpo_synchronized = true;
        break;
    case GGPO_EVENTCODE_DISCONNECTED_FROM_PEER:
        g_status_msg = "DISCONNECTED";
        g_disconnected = true;
        break;
    case GGPO_EVENTCODE_TIMESYNC:
        g_timesync_frames = info->u.timesync.frames_ahead;
        g_status_msg = "TIMESYNC " + std::to_string(g_timesync_frames);
        break;
    }
    return true;
}

}

void prepare()
{
    g_frame_base = g_game->GetState().match.frame.get();
    g_saved_state_map.clear();
    g_manual_frame_advance_enabled_backup = g_cfg->get_manual_frame_advance_settings().enabled;
    g_cfg->get_manual_frame_advance_settings().enabled = false;
}

void start_ggpo_synctest()
{
    assert(g_session == nullptr);

    prepare();

    GGPOSessionCallbacks callbacks;
    callbacks.advance_frame = ggpo_callbacks::advance_frame;
    callbacks.begin_game = ggpo_callbacks::begin_game;
    callbacks.free_buffer = ggpo_callbacks::free_buffer;
    callbacks.load_game_state = ggpo_callbacks::load_game_state;
    callbacks.log_game_state = ggpo_callbacks::log_game_state;
    callbacks.on_event = ggpo_callbacks::on_event;
    callbacks.save_game_state = ggpo_callbacks::save_game_state;
    GGPO_CHECK(ggpo_start_synctest(&g_session, &callbacks, "GGXX", 2, 2, g_cfg->get_args().synctest_frames));

    GGPOPlayer player;
    player.player_num = 1;
    player.type = GGPO_PLAYERTYPE_LOCAL;
    player.size = sizeof(GGPOPlayer);
    GGPO_CHECK(ggpo_add_player(g_session, &player, &g_player_handles[0]));
    player.player_num++;
    GGPO_CHECK(ggpo_add_player(g_session, &player, &g_player_handles[1]));
}

void start_ggpo_session()
{
    assert(g_session == nullptr);

    prepare();

    WSADATA wd = { 0 };
    ::WSAStartup(MAKEWORD(2, 2), &wd);

    const auto& network_args = g_cfg->get_args().network;

    GGPOSessionCallbacks callbacks;
    callbacks.advance_frame = ggpo_callbacks::advance_frame;
    callbacks.begin_game = ggpo_callbacks::begin_game;
    callbacks.free_buffer = ggpo_callbacks::free_buffer;
    callbacks.load_game_state = ggpo_callbacks::load_game_state;
    callbacks.log_game_state = ggpo_callbacks::log_game_state;
    callbacks.on_event = ggpo_callbacks::on_event;
    callbacks.save_game_state = ggpo_callbacks::save_game_state;
    GGPO_CHECK(ggpo_start_session(&g_session, &callbacks, "GGXX", 2, 2, network_args.localport));

    for (uint8_t player_num = 1; player_num <= 2; ++player_num)
    {
        GGPOPlayer player;
        player.player_num = player_num;
        player.size = sizeof(GGPOPlayer);
        if (network_args.side == player_num)
        {
            player.type = GGPO_PLAYERTYPE_LOCAL;
        }
        else
        {
            player.type = GGPO_PLAYERTYPE_REMOTE;
            std::copy(network_args.remoteip.begin(), network_args.remoteip.end(), player.u.remote.ip_address);
            player.u.remote.ip_address[network_args.remoteip.size()] = 0;
            player.u.remote.port = network_args.remoteport;
            g_status_msg = "CONNECTING " + std::string(player.u.remote.ip_address) + ":" + std::to_string((int)player.u.remote.port);
        }
        GGPO_CHECK(ggpo_add_player(g_session, &player, &g_player_handles[player_num-1]));
    }

    ggpo_set_disconnect_timeout(g_session, 3000);
    ggpo_set_disconnect_notify_start(g_session, 1000);
}

void close_session()
{
    assert(g_session);

    g_cfg->get_manual_frame_advance_settings().enabled = g_manual_frame_advance_enabled_backup;

    GGPO_CHECK(ggpo_close_session(g_session));
    g_session = nullptr;

    if (g_network_enabled)
    {
        ::WSACleanup();
    }

    g_saved_state_map.clear();
    g_network_enabled = false;
    g_disconnected = false;
    g_status_msg = "SESSION CLOSED";
    g_timesync_frames = 0;
    g_ggpo_synchronized = false;
}

void display_status(IGame* game, const char* status)
{
    game->WriteCockpitFont(status, 260, 460, 1, 0xff, 1);
}

bool g_recursive_guard = false;
bool after_read_input(IGame* game)
{
    if (!g_session)
        return true;

    LIBGG_LOG() << "g_recursive_guard=" << g_recursive_guard << std::endl;

    if (g_recursive_guard)
        return true;

    g_recursive_guard = true;

    IGame::input_t timesync_input{};
    if (g_ggpo_synchronized && g_timesync_frames > 0 && (g_game->GetState().match.frame.get() % 7 == 0))
    {
        timesync_input = game->RemapInputToDefault(game->GetCachedInput());
        g_status_msg = "TIMESYNC " + std::to_string(g_timesync_frames);
        game->DrawFrame();
        while (game->Idle()) {}
        game->ReadInput();
        --g_timesync_frames;
    }

    IGame::input_t input = game->RemapInputToDefault(game->GetCachedInput());
    input[0] |= timesync_input[0];
    input[1] |= timesync_input[1];

    const auto& network_args = g_cfg->get_args().network;
    if (network_args.side == 2)
        std::swap(input[0], input[1]);

    GGPOErrorCode result = GGPO_OK;
    for (uint8_t side = 1; side <= 2; ++side)
    {
        if (!g_network_enabled || network_args.side == side)
        {
            result = ggpo_add_local_input(g_session, g_player_handles[side-1], (void*)&input[side-1], 2);
        }
    }

    if (result != GGPO_ERRORCODE_NOT_SYNCHRONIZED && result != GGPO_ERRORCODE_PREDICTION_THRESHOLD)
    {
        GGPO_CHECK(result);
        int disconnected = 0;
        GGPO_CHECK(ggpo_synchronize_input(g_session, (void*)&input, 4, &disconnected));
        if (disconnected)
            g_disconnected = true;
    }
    else
    {
        input = {};
        game->DrawFrame();
        game->AbortCurrentFrame();
    }

    game->SetCachedInput(game->RemapInputFromDefault(input));

    g_recursive_guard = false;

    return true;
}

// this callback can be called recursively, but no more than 2 calls deep
bool after_advance_frame(IGame*)
{
    if (g_session)
    {
        LIBGG_LOG() << std::endl;
        GGPO_CHECK(ggpo_advance_frame(g_session));
    }

    g_status_displayed = false;

    return true;
}

bool idle(IGame* game)
{
    if (!g_session)
        return true;
    int timeout = std::clamp(game->MsTillNextFrame(), 0, 3);
    LIBGG_LOG() << "timeout=" << timeout << std::endl;
    if (timeout > 0)
        ggpo_idle(g_session, timeout);
    return true;
}

bool before_draw_frame(IGame* game)
{
    if (!g_status_displayed && !g_status_msg.empty())
    {
        g_status_displayed = true;
        display_status(game, g_status_msg.c_str());
    }

    if (!g_session)
        return true;

    LIBGG_LOG() << std::endl;

    if (g_ggpo_synchronized && game->GetState().match.frame.get() % 60 == 0)
    {
        auto side = g_cfg->get_args().network.side;
        GGPONetworkStats stats{};
        GGPO_CHECK(ggpo_get_network_stats(g_session, g_player_handles[side == 1 ? 1: 0], &stats));
        g_status_msg = "PING " + std::to_string(stats.network.ping);
    }
    return true;
}

bool after_draw_frame(IGame*)
{
    if (g_recursive_guard)
    {
        // called by during ReadInput callback
        // close_session should not be called during ReadInput
        return true;
    }

    LIBGG_LOG() << std::endl;

    if (!g_session)
    {
        if (g_network_enabled)
            start_ggpo_session();
        else if (g_cfg->get_args().synctest_frames)
            start_ggpo_synctest();
    }
    else
    {
        if (g_disconnected)
            close_session();
    }

    return true;
}

}

void Initialize(IGame* game, configuration* cfg)
{
    game->RegisterCallback(IGame::Event::AfterReadInput, after_read_input);
    game->RegisterCallback(IGame::Event::AfterAdvanceFrame, after_advance_frame);
    game->RegisterCallback(IGame::Event::BeforeDrawFrame, before_draw_frame);
    game->RegisterCallback(IGame::Event::AfterDrawFrame, after_draw_frame);
    game->RegisterCallback(IGame::Event::Idle, idle);
    apply_patches(game->GetImageBase());
    g_game = game;
    g_cfg = cfg;
}

}
