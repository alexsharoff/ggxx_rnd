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


#define GGPO_CHECK(expr) \
    do { \
        const auto res___ = expr; \
        if (!GGPO_SUCCEEDED(res___)) { \
            std::cerr << __FILE__ << ':' << __LINE__ << ": " << #expr << " == " << res___; \
            throw std::logic_error("ggpo error"); \
        } \
    } while(false)

#pragma once


namespace ggpo
{

namespace
{

GGPOSession* g_session = nullptr;
GGPOPlayerHandle g_player_handles[2];
bool g_call_ggpo_idle_manually = true;
bool g_replaying_input = false;
size_t g_frame_base = 0;
std::unordered_map<size_t, std::shared_ptr<game_state>> g_saved_state_map;
size_t g_vs_2p_jmp_addr = 0;
IGame* g_game = nullptr;
configuration* g_cfg = nullptr;
bool g_drawing_enabled = true;


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
    //TODO: start_ggpo_session()
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
    LIBGG_LOG() << std::endl;
    return true;
}

bool save_game_state(unsigned char **buffer, int *len, int *checksum, int frame)
{
    LIBGG_LOG() << std::endl;

    auto state_ptr = std::make_shared<game_state>(g_game->GetState());
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
    LIBGG_LOG() << std::endl;
    auto state_ptr = (game_state*)(buffer);
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

    LIBGG_LOG() << std::endl;

    auto state = (game_state*)buffer;
    auto frame = state->match2.frame.get() - g_frame_base;
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
    g_replaying_input = true;
    g_game->GameTick();
    if (g_drawing_enabled)
        g_game->EnableDrawing(true);
    g_replaying_input = false;
    return true;
}

bool on_event(GGPOEvent *info)
{
    LIBGG_LOG() << info->code << std::endl;
    return true;
}

}

void start_synctest()
{
    assert(g_session == nullptr);

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

    g_call_ggpo_idle_manually = true;
    g_frame_base = g_game->GetState().match2.frame.get();

    g_saved_state_map.clear();

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

    // ggpo_start_synctest doesn't suppot delay
    //GGPO_CHECK(ggpo_set_frame_delay(g_session, g_player_handles[0], 6));
    //GGPO_CHECK(ggpo_set_frame_delay(g_session, g_player_handles[1], 6));

    //ggpo_set_disconnect_notify_start(g_session, 1000);
    //ggpo_set_disconnect_timeout(g_session, 10000);

    LIBGG_LOG() << "end" << std::endl;
}

void close_session()
{
    assert(g_session);

    if (g_drawing_enabled)
        g_game->EnableFpsLimit(true);

    GGPO_CHECK(ggpo_close_session(g_session));
    g_session = nullptr;

    g_saved_state_map.clear();

    LIBGG_LOG() << "end" << std::endl;
}

bool input_data_hook(IGame* game)
{
    if (!g_session)
        return true;

    auto input = game->GetInputRemapped();

    LIBGG_LOG() << std::endl;

    if (!g_replaying_input && g_drawing_enabled)
    {
        game->EnableFpsLimit(true);
        game->LimitFps();
        game->EnableFpsLimit(false);
    }

    if (g_call_ggpo_idle_manually)
        GGPO_CHECK(ggpo_idle(g_session, 1));

    g_call_ggpo_idle_manually = true;

    const auto frame = g_game->GetState().match2.frame.get() - g_frame_base;
    const auto found = g_saved_state_map.find(frame);
    if (found == g_saved_state_map.end())
    {
        g_saved_state_map[frame] = std::make_shared<game_state>(game->GetState());
    }

    GGPO_CHECK(ggpo_add_local_input(g_session, g_player_handles[0], (void*)&input[0], 2));
    GGPO_CHECK(ggpo_add_local_input(g_session, g_player_handles[1], (void*)&input[1], 2));
    int disconnected = 0;
    GGPO_CHECK(ggpo_synchronize_input(g_session, (void*)&input, 4, &disconnected));

    game->SetInputRemapped(input);

    return true;
}

bool game_tick_end_hook(IGame*)
{
    if (!g_session)
    {
        if (g_cfg->get_args().synctest_frames)
        {
            start_synctest();
        }
        return true;
    }

    LIBGG_LOG() << "advance_frame" <<  std::endl;
    GGPO_CHECK(ggpo_advance_frame(g_session));

    return true;
}

bool sleep_hook(IGame* game)
{
    if (!g_session)
        return true;

    LIBGG_LOG() << std::endl;

    assert(!g_replaying_input);

    const auto ms = game->GetSleepTime();
    using std::chrono::steady_clock;
    const auto idle_begin = steady_clock::now();
    GGPO_CHECK(ggpo_idle(g_session, ms));
    const auto idle_end = steady_clock::now();

    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    const auto idle_ms = duration_cast<milliseconds>(idle_end - idle_begin).count();
    if (ms > idle_ms)
        ::Sleep(static_cast<DWORD>(ms - idle_ms));

    g_call_ggpo_idle_manually = false;

    return true;
}

}

void Initialize(IGame* game, configuration* cfg)
{
    game->RegisterCallback(IGame::Event::AfterGetInput, input_data_hook);
    game->RegisterCallback(IGame::Event::BeforeSleep, sleep_hook);
    game->RegisterCallback(IGame::Event::AfterGameTick, game_tick_end_hook);
    apply_patches(game->GetImageBase());
    g_drawing_enabled = game->IsDrawingEnabled();
    g_game = game;
    g_cfg = cfg;
}

}
