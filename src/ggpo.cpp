#include "ggpo.h"

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
            throw std::logic_error("ggpo error"); \
        } \
    } while(false)

namespace ggpo
{

namespace
{

GGPOSession* g_session = nullptr;
GGPOPlayerHandle g_player_handles[2];
bool g_ggpo_frame_advance = false;
size_t g_frame_base = 0;
std::unordered_map<size_t, std::shared_ptr<game_state>> g_saved_state_map;
size_t g_vs_2p_jmp_addr = 0;
IGame* g_game = nullptr;
configuration* g_cfg = nullptr;
bool g_drawing_enabled = true;
bool g_manual_frame_advance_enabled_backup = false;
bool g_network_enabled = false;
bool g_disconnected = false;
std::string g_status_msg = "";
std::optional<uint32_t> g_restore_frame;
uint32_t g_timesync_frames = 0;


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
    LIBGG_LOG() << state_ptr->match2.frame.get() << std::endl;
    assert(state_ptr->match2.frame.get() - g_frame_base == static_cast<size_t>(frame));

    *buffer = (unsigned char*)state_ptr.get();
    *len = sizeof(game_state);
    *checksum = state_checksum(*state_ptr);

    if (g_cfg->get_args().synctest_frames > 0)
    {
        auto found = g_saved_state_map.find(frame);
        if (found != g_saved_state_map.end())
        {
            int checksum_old = state_checksum(*found->second);
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
    LIBGG_LOG() << state_ptr->match2.frame.get() << std::endl;
    g_game->SetState(*state_ptr);
    g_restore_frame.reset();
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
    auto frame = state->match2.frame.get() - g_frame_base;
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
    if (g_drawing_enabled)
        g_game->EnableDrawing(false);
    g_ggpo_frame_advance = true;
    g_game->GameTick();
    if (g_drawing_enabled)
        g_game->EnableDrawing(true);
    g_ggpo_frame_advance = false;
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
        break;
    case GGPO_EVENTCODE_RUNNING:
        g_status_msg = "";
        break;
    case GGPO_EVENTCODE_CONNECTION_INTERRUPTED:
        g_status_msg = "CONNECTION INTERRUPTED";
        break;
    case GGPO_EVENTCODE_CONNECTION_RESUMED:
        g_status_msg = "CONNECTION RESUMED";
        break;
    case GGPO_EVENTCODE_DISCONNECTED_FROM_PEER:
        g_status_msg = "DISCONNECTED";
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
    // ggpo_idle() must be called instead of Sleep().
    // In Steam release Sleep() is called just before IDirect3D::Present,
    // but sometimes it's skipped, which is a problem.
    // To be consistent with recommended implementation (VectorWar),
    // ggpo_idle() should be called at the beginning of game tick,
    // or at least before doing any processing for current frame.
    // TODO: research, will this change game timing?
    // 
    // This disables limit_fps() function when it's called by the game.
    // We will call it manually after getting current input data.
    g_game->EnableFpsLimit(false);

    g_frame_base = g_game->GetState().match2.frame.get();

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

    g_game->EnableFpsLimit(g_drawing_enabled);
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
    g_status_msg = "";
    g_timesync_frames = 0;
}

void limit_fps(IGame* game)
{
    game->EnableFpsLimit(true);
    game->LimitFps();
    game->EnableFpsLimit(false);
}

bool input_data_hook(IGame* game)
{
    if (!g_session)
        return true;

    IGame::input_t timesync_input{};
    if (!g_ggpo_frame_advance && g_drawing_enabled)
    {
        limit_fps(game);
        if (g_timesync_frames > 0 && (g_game->GetState().match2.frame.get() % 7 == 0))
        {
            timesync_input = game->GetInputRemapped();
            limit_fps(game);
            --g_timesync_frames;
            if (g_timesync_frames > 0)
                g_status_msg = "TIMESYNC " + std::to_string(g_timesync_frames);
            else
                g_status_msg.clear();
        }
    }

    auto input = game->GetInputRemapped();
    input[0] |= timesync_input[0];
    input[1] |= timesync_input[1];

    if (g_restore_frame.has_value())
    {
        g_game->SetState(*g_saved_state_map.at(*g_restore_frame));
    }

    const auto& network_args = g_cfg->get_args().network;
    if (network_args.side == 2)
        std::swap(input[0], input[1]);

    // timeout argument isn't used for anything useful,
    // so let's just set it to 0.
    GGPO_CHECK(ggpo_idle(g_session, 0));

    const auto frame = g_game->GetState().match2.frame.get() - g_frame_base;
    const auto found = g_saved_state_map.find(frame);
    if (found == g_saved_state_map.end())
    {
        g_saved_state_map[frame] = std::make_shared<game_state>(game->GetState());
    }

    GGPOErrorCode result = GGPO_OK;
    if (!g_ggpo_frame_advance)
    {
        for (uint8_t side = 1; side <= 2; ++side)
        {
            if (!g_network_enabled || network_args.side == side)
            {
                result = ggpo_add_local_input(g_session, g_player_handles[side-1], (void*)&input[side-1], 2);
            }
        }
    }
    if (GGPO_SUCCEEDED(result))
    {
        int disconnected = 0;
        result = ggpo_synchronize_input(g_session, (void*)&input, 4, &disconnected);
        if (GGPO_SUCCEEDED(result) && !g_ggpo_frame_advance)
        {
            LIBGG_LOG() << "ggpo_synchronize_input" << std::endl;
            g_restore_frame.reset();
        }
        if (disconnected)
            g_disconnected = true;
    }

    if (!GGPO_SUCCEEDED(result))
    {
        if (!g_restore_frame.has_value())
        {
            g_restore_frame = frame;
            LIBGG_LOG() << "g_restore_frame=" << frame << std::endl;
        }
        input = IGame::input_t{};
    }

    game->SetInputRemapped(input);

    return true;
}

bool game_tick_end_hook(IGame*)
{
    if (!g_session)
    {
        if (g_network_enabled)
        {
            start_ggpo_session();
        }
        else if (g_cfg->get_args().synctest_frames)
        {
            start_ggpo_synctest();
        }
        return true;
    }

    if (!g_restore_frame.has_value() || !g_network_enabled || g_ggpo_frame_advance)
    {
        LIBGG_LOG() << "advance_frame" <<  std::endl;
        GGPO_CHECK(ggpo_advance_frame(g_session));
    }

    if (!g_ggpo_frame_advance)
    {
        if (g_disconnected)
        {
            close_session();
            return true;
        }
    }

    return true;
}

bool process_objects_hook(IGame* game)
{
    if (!g_status_msg.empty())
    {
        game->WriteCockpitFont(g_status_msg.c_str(), 260, 460, 1, 0xff, 1);
    }
    return true;
}

}

void Initialize(IGame* game, configuration* cfg)
{
    game->RegisterCallback(IGame::Event::AfterGetInput, input_data_hook);
    game->RegisterCallback(IGame::Event::AfterGameTick, game_tick_end_hook);
    game->RegisterCallback(IGame::Event::AfterProcessObjects, process_objects_hook);
    apply_patches(game->GetImageBase());
    g_drawing_enabled = game->IsDrawingEnabled();
    g_game = game;
    g_cfg = cfg;
}

}
