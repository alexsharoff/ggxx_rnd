#include "network.h"

#include <ggponet.h>

#include <chrono>
#include <iostream>
#include <unordered_map>
#include <unordered_set>


#define GGPO_CHECK(expr) \
    do { \
        const auto res___ = expr; \
        if (!GGPO_SUCCEEDED(res___)) { \
            std::cerr << __FILE__ << ':' << __LINE__ << ": " << #expr << " == " << res___; \
            throw std::logic_error("ggpo error"); \
        } \
    } while(false)

namespace network
{

GGPOSession* g_session = nullptr;
GGPOPlayerHandle g_player_handles[2];
bool g_call_ggpo_idle_manually = true;
std::unordered_map<const IXACT3WaveBank*, std::unordered_set<int16_t>> g_heard_sounds;

bool begin_game(const char*)
{
    return true;
}

int g_counter = 0;
bool save_game_state(unsigned char **buffer, int *len, int *checksum, int frame)
{
    *buffer = new unsigned char[4];
    const auto begin = (unsigned char*)&g_counter;
    std::copy(begin, begin + 4, *buffer);
    ++g_counter;
    *len = 4;
    std::cout << "save_game_state" << std::endl;
    return true;
}

bool load_game_state(unsigned char *buffer, int len)
{
    std::cout << "load_game_state" << std::endl;
    return true;
}

bool log_game_state(char *filename, unsigned char *buffer, int len)
{
    return true;
}

void free_buffer(void *buffer)
{
    // Free'd all buffers => enable_round_end_condition(true)
    delete[] buffer;
}

bool advance_frame(int)
{
    enable_drawing(false);
    g_capture_game_state = false;
    ::game_tick();
    enable_drawing(true);
    g_capture_game_state = true;
    return true;
}

bool on_event(GGPOEvent *info)
{
    std::cout << info->code << std::endl;
    return true;
}

void start_session()
{
    if (g_session)
        return;

    GGPOSessionCallbacks callbacks;
    callbacks.advance_frame = advance_frame;
    callbacks.begin_game = begin_game;
    callbacks.free_buffer = free_buffer;
    callbacks.load_game_state = load_game_state;
    callbacks.log_game_state = log_game_state;
    callbacks.on_event = on_event;
    callbacks.save_game_state = save_game_state;
    GGPO_CHECK(ggpo_start_synctest(&g_session, &callbacks, "GGXX", 2, 2, 1));

    GGPOPlayer player;
    player.player_num = 0;
    player.type = GGPO_PLAYERTYPE_LOCAL;
    player.size = sizeof(GGPOPlayer);
    GGPO_CHECK(ggpo_add_player(g_session, &player, &g_player_handles[0]));
    player.player_num = 1;
    GGPO_CHECK(ggpo_add_player(g_session, &player, &g_player_handles[1]));

    GGPO_CHECK(ggpo_set_frame_delay(g_session, g_player_handles[0], 0));
    GGPO_CHECK(ggpo_set_frame_delay(g_session, g_player_handles[1], 0));

    //ggpo_set_disconnect_notify_start(g_session, 1000);
    //ggpo_set_disconnect_timeout(g_session, 10000);

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
    g_enable_fps_limit = false;

    // Round end condition should be enabled only after we receive
    // last input from the remote side.
    enable_round_end_condition(false);

    // During rollback, don't play sound that were already heard 
    g_heard_sounds.clear();
}

void close_session()
{
    if (g_session)
    {
        GGPO_CHECK(ggpo_close_session(g_session));
        g_session = nullptr;
    }
}

namespace callbacks
{

bool raw_input_data(input_data& input)
{
    if (!g_session)
        return false;

    g_enable_fps_limit = true;
    limit_fps();
    g_enable_fps_limit = false;

    if (g_call_ggpo_idle_manually)
        GGPO_CHECK(ggpo_idle(g_session, 1));
    else
        g_call_ggpo_idle_manually = true;

    GGPO_CHECK(ggpo_add_local_input(g_session, g_player_handles[0], (void*)&input.keys[0], 2));
    GGPO_CHECK(ggpo_add_local_input(g_session, g_player_handles[1], (void*)&input.keys[1], 2));
    int disconnected = 0;
    input = input_data();
    GGPO_CHECK(ggpo_synchronize_input(g_session, (void*)input.keys, 4, &disconnected));

    return true;
}

bool game_tick_end()
{
    if (!g_session)
        return false;

    GGPO_CHECK(ggpo_advance_frame(g_session));

    return true;
}

bool sleep(uint32_t ms)
{
    if (!g_session)
        return false;

    using std::chrono::steady_clock;
    const auto idle_begin = steady_clock::now();
    GGPO_CHECK(ggpo_idle(g_session, ms));
    const auto idle_end = steady_clock::now();

    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    const auto idle_ms = duration_cast<milliseconds>(idle_end - idle_begin).count();
    if (ms > idle_ms)
        ::Sleep(ms - idle_ms);

    g_call_ggpo_idle_manually = false;

    return true;
}

// Stub implementation: play each sound once
bool play_sound(const IXACT3WaveBank* bank, int16_t sound_id)
{
    if (!g_session)
        return false;

    auto [_, success] = g_heard_sounds[bank].insert(sound_id);
    if (success)
        return false;

    return true;
}

}

}
