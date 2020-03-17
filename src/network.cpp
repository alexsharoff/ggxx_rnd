#include "network.h"

#include <ggponet.h>

#include <chrono>


namespace network
{

// TODO: transfer additional state: selected chars, rng etc

GGPOSession* g_session = nullptr;
GGPOPlayerHandle g_player_handles[2];

bool begin_game(const char*)
{
    return true;
}

bool save_game_state(unsigned char **buffer, int *len, int *checksum, int frame)
{
    return true;
}

bool load_game_state(unsigned char *buffer, int len)
{
    return true;
}

bool log_game_state(char *filename, unsigned char *buffer, int len)
{
    return true;
}

void free_buffer(void *buffer)
{
}

bool advance_frame(int flags)
{
    // CaptureState(false)
    // DisableDrawing(false)
    // ggpo_synchronize_input
    // game_tick
    // ggpo_advance_frame
    return true;
}

bool on_event(GGPOEvent *info)
{
    return true;
}

void start()
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
    ggpo_start_synctest(&g_session, &callbacks, "GGXX", 2, 2, 1);

    GGPOPlayer player;
    player.player_num = 0;
    player.type = GGPO_PLAYERTYPE_LOCAL;
    player.size = sizeof(GGPOPlayer);
    ggpo_add_player(g_session, &player, &g_player_handles[0]);
    player.player_num = 1;
    ggpo_add_player(g_session, &player, &g_player_handles[1]);

    ggpo_set_frame_delay(g_session, g_player_handles[0], 0);
    ggpo_set_frame_delay(g_session, g_player_handles[1], 0);

    //ggpo_set_disconnect_notify_start(g_session, 1000);
    //ggpo_set_disconnect_timeout(g_session, 10000);
}

void stop()
{
    if (g_session)
    {
        ggpo_close_session(g_session);
        g_session = nullptr;
    }
}

void process_input(const input_data& input)
{
    ggpo_add_local_input(g_session, g_player_handles[0], (void*)&input.keys[0], 2);
    ggpo_add_local_input(g_session, g_player_handles[1], (void*)&input.keys[1], 2);
    int disconnected = 0;
    ggpo_synchronize_input(g_session, (void*)input.keys, 4, &disconnected);
}

void game_tick_end(char game_state, int idle_timeout_ms)
{
    using std::chrono::steady_clock;
    const auto idle_begin = steady_clock::now();
    ggpo_idle(g_session, idle_timeout_ms);
    const auto idle_end = steady_clock::now();

    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    const auto idle_ms = duration_cast<milliseconds>(idle_end - idle_begin).count();
    if (idle_timeout_ms > idle_ms)
        sleep(idle_timeout_ms - idle_ms);

    ggpo_advance_frame(g_session);
}

}
