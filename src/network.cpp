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
struct pair_hash
{
    template<class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2>& pair) const
    {
        return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
    }
};
using sound_set_t = std::unordered_set<std::pair<const IXACT3WaveBank*, int16_t>, pair_hash>;
std::unordered_map<size_t, sound_set_t> g_heard_sounds;
bool g_during_match = false;
bool g_replaying_input = false;
bool g_is_active = false;
size_t g_frame = 0;
bool g_is_waiting = false;

void activate()
{
    g_is_active = true;
    g_is_waiting = true;
}

bool begin_game(const char*)
{
    std::cout << "begin_game" << std::endl;
    return true;
}

bool save_game_state(unsigned char **buffer, int *len, int *checksum, int frame)
{
    *buffer = new unsigned char[4];
    const auto begin = (unsigned char*)&g_frame;
    std::copy(begin, begin + 4, *buffer);
    *len = 4;
    *checksum = g_frame;
    std::cout << "save_game_state: " << g_frame << std::endl;
    return true;
}

bool load_game_state(unsigned char *buffer, int len)
{
    g_frame = *(size_t*)buffer;
    std::cout << "load_game_state: " << g_frame <<  std::endl;
    return true;
}

bool log_game_state(char *filename, unsigned char *buffer, int len)
{
    return true;
}

void free_buffer(void *buffer)
{
    if (!buffer)
        return;
    const auto frame = *(size_t*)buffer;
    std::cout << "free_buffer: " << frame <<  std::endl;
    auto found = g_heard_sounds.find(frame);
    if (found != g_heard_sounds.end())
        g_heard_sounds.erase(found);

    // Free'd all buffers => enable_round_end_condition(true)
    delete[] buffer;
}

bool advance_frame(int)
{
    std::cout << "advance_frame" <<  std::endl;
    enable_drawing(false);
    g_replaying_input = true;
    ::game_tick();
    enable_drawing(true);
    g_replaying_input = false;
    return true;
}

bool on_event(GGPOEvent *info)
{
    std::cout << "event: " << info->code << std::endl;
    return true;
}

void on_match_start()
{
    std::cout << "on_match_start" << std::endl;

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

    // Round end condition should be enabled only after we confirm
    // last input from the remote side.
    enable_round_end_condition(false);

    enable_pause_menu(false);

    g_during_match = true;
    g_call_ggpo_idle_manually = true;
    g_frame = 0;
}

void on_match_end()
{
    std::cout << "on_match_end" << std::endl;

    g_enable_fps_limit = true;
    enable_round_end_condition(true);
    enable_pause_menu(true);
    g_during_match = false;
    g_heard_sounds.clear();
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

    // During rollback, don't play sounds that were already heard 
    g_heard_sounds.clear();

    g_during_match = false;
    g_is_waiting = true;

    std::cout << "start_session" << std::endl;
}

void close_session()
{
    if (g_session)
    {
        GGPO_CHECK(ggpo_close_session(g_session));
        g_session = nullptr;
    }

    std::cout << "close_session" << std::endl;
}

namespace callbacks
{

bool raw_input_data(input_data& input)
{
    if (!g_session || !g_during_match)
        return false;

    std::cout << "raw_input_data" <<  std::endl;

    if (!g_replaying_input)
    {
        g_enable_fps_limit = true;
        limit_fps();
        g_enable_fps_limit = false;
    }

    if (g_call_ggpo_idle_manually)
        GGPO_CHECK(ggpo_idle(g_session, 1));

    g_call_ggpo_idle_manually = true;

    GGPO_CHECK(ggpo_add_local_input(g_session, g_player_handles[0], (void*)&input.keys[0], 2));
    GGPO_CHECK(ggpo_add_local_input(g_session, g_player_handles[1], (void*)&input.keys[1], 2));
    int disconnected = 0;
    input = input_data();
    GGPO_CHECK(ggpo_synchronize_input(g_session, (void*)input.keys, 4, &disconnected));

    return true;
}

bool game_tick_begin()
{
    if (!g_is_active)
        return false;

    if (in_match())
    {
        g_is_waiting = false;
        if (!g_during_match)
        {
            start_session();
            std::cout << "game_tick_begin: on_match_start" <<  std::endl;
            on_match_start();
        }
    }
    else
    {
        if (!g_replaying_input)
        {
            if(g_during_match)
            {
                std::cout << "game_tick_begin: on_match_end" <<  std::endl;
                on_match_end();
                close_session();
            }

            // Exited from VS 2P
            if (!g_is_waiting && (find_fiber_by_name("OPTION")))
            {
                std::cout << "game_tick_begin: close_session" <<  std::endl;
                on_match_end();
                close_session();
                g_is_active = false;
            }
        }
    }

    return g_session != nullptr;
}

bool game_tick_end()
{
    if (!g_session || !g_during_match)
        return false;

    std::cout << "game_tick_end" <<  std::endl;

    ++g_frame;

    GGPO_CHECK(ggpo_advance_frame(g_session));

    return true;
}

bool sleep(uint32_t ms)
{
    if (!g_session || !g_during_match)
        return false;

    std::cout << "sleep" <<  std::endl;

    assert(!g_replaying_input);

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

bool play_sound(const IXACT3WaveBank* bank, int16_t sound_id)
{
    if (!g_session || !g_during_match)
        return false;

    std::cout << "play_sound" <<  std::endl;

    auto [_, success] = g_heard_sounds[g_frame].insert({bank, sound_id});
    if (success)
        return false;

    return true;
}

}

}
