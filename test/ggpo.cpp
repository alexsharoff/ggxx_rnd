#include <ggponet.h>

#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

#include <Windows.h>
#include <timeapi.h>
#include <Winsock2.h>


#define GGPO_CHECK(expr) \
    do { \
        const auto res___ = expr; \
        if (!GGPO_SUCCEEDED(res___)) { \
            std::cerr << __FILE__ << ':' << __LINE__ << ": " << #expr << " == " << res___ << std::endl; \
            throw std::logic_error("GGPO error"); \
        } \
    } while(false)


using input_t = std::array<uint8_t, 4>;

class GameState
{
public:
    GameState(int side, const std::vector<input_t>& input, uint16_t localport, const std::string_view& remoteip, uint16_t remoteport)
    {
        GGPOSessionCallbacks callbacks;
        if (side == 1)
        {
            callbacks.advance_frame = Callbacks::advance_frame_p1;
            callbacks.on_event = Callbacks::on_event_p1;
            callbacks.load_game_state = Callbacks::load_game_state_p1;
            callbacks.save_game_state = Callbacks::save_game_state_p1;
        }
        else
        {
            callbacks.advance_frame = Callbacks::advance_frame_p2;
            callbacks.on_event = Callbacks::on_event_p2;
            callbacks.load_game_state = Callbacks::load_game_state_p2;
            callbacks.save_game_state = Callbacks::save_game_state_p2;
        }
        callbacks.free_buffer = Callbacks::free_buffer;
        callbacks.begin_game = Callbacks::begin_game;
        callbacks.log_game_state = Callbacks::log_game_state;
        GGPO_CHECK(ggpo_start_session(&m_session, &callbacks, "TEST", 2, 2, localport));

        for (int player_num = 1; player_num <= 2; ++player_num)
        {
            GGPOPlayer player;
            player.player_num = player_num;
            player.size = sizeof(GGPOPlayer);
            if (side == player_num)
            {
                player.type = GGPO_PLAYERTYPE_LOCAL;
            }
            else
            {
                player.type = GGPO_PLAYERTYPE_REMOTE;
                std::copy(remoteip.begin(), remoteip.end(), player.u.remote.ip_address);
                player.u.remote.ip_address[remoteip.size()] = 0;
                player.u.remote.port = remoteport;
            }
            GGPO_CHECK(ggpo_add_player(m_session, &player, &m_player_handles[player_num-1]));
        }

        m_input = input;
        m_side = side;
    }

    ~GameState()
    {
        ggpo_close_session(m_session);
    }

    bool advance_frame(const input_t input[2], int disconnect_flags)
    {
        (void)input;
        (void)disconnect_flags;
        ggpo_advance_frame(m_session);
        return true;
    }

    bool save_game_state(unsigned char** buffer, int* len , int* checksum, int /* frame */) const
    {
        *buffer = new unsigned char[4];
        const char* test = "test";
        std::copy(test, test + 4, *buffer);
        *len = 4;
        *checksum = reinterpret_cast<int>(this);

        return true;
    }

    static void free_buffer(void* buffer)
    {
        delete[] buffer;
    }

    bool load_game_state(unsigned char* /* buffer */, int /* len */)
    {
        return true;
    }

    bool on_event(GGPOEvent* info)
    {
        switch (info->code)
        {
        case GGPO_EVENTCODE_CONNECTED_TO_PEER:
            break;
        case GGPO_EVENTCODE_SYNCHRONIZING_WITH_PEER:
            break;
        case GGPO_EVENTCODE_SYNCHRONIZED_WITH_PEER:
            break;
        case GGPO_EVENTCODE_RUNNING:
            break;
        case GGPO_EVENTCODE_CONNECTION_INTERRUPTED:
            break;
        case GGPO_EVENTCODE_CONNECTION_RESUMED:
            break;
        case GGPO_EVENTCODE_DISCONNECTED_FROM_PEER:
            break;
        case GGPO_EVENTCODE_TIMESYNC:
            m_timesync_frames = info->u.timesync.frames_ahead;
            break;
        }
        return true;
    }

    bool input_ended() const
    {
        return m_input.size() <= m_input_idx;
    }

    void idle(uint64_t timeout_ms) const
    {
        ggpo_idle(m_session, static_cast<int>(timeout_ms));
    }

    void run_frame()
    {
        auto local_input = m_input[m_input_idx++];
        auto result = ggpo_add_local_input(
            m_session, m_player_handles[m_side-1], &local_input, sizeof(local_input)
        );
        if (GGPO_SUCCEEDED(result))
        {
            int disconnect_flags;
            input_t inputs[2];
            result = ggpo_synchronize_input(m_session, inputs, sizeof(inputs), &disconnect_flags);
            if (GGPO_SUCCEEDED(result))
                advance_frame(inputs, disconnect_flags);
        }
        draw_current_frame();
    }

    void draw_current_frame() const
    {
    }

private:
    GGPOSession* m_session;
    GGPOPlayerHandle m_player_handles[2];
    int m_side;
    int m_timesync_frames = 0;
    std::vector<input_t> m_input;
    size_t m_input_idx = 0;

    GGPOSession* get_session() const
    {
        return m_session;
    }

    struct Callbacks
    {
        static GameState* g_game_state_p1;
        static GameState* g_game_state_p2;
        static bool begin_game(const char*)
        {
            return true;
        }

        static bool save_game_state(GameState& game, unsigned char** buffer, int* len, int* checksum, int frame)
        {
            return game.save_game_state(buffer, len, checksum, frame);
        }

        static bool save_game_state_p1(unsigned char** buffer, int* len, int* checksum, int frame)
        {
            return save_game_state(*g_game_state_p1, buffer, len, checksum, frame);
        }

        static bool save_game_state_p2(unsigned char** buffer, int* len, int* checksum, int frame)
        {
            return save_game_state(*g_game_state_p2, buffer, len, checksum, frame);
        }

        static bool load_game_state(GameState& game, unsigned char* buffer, int len)
        {
            return game.load_game_state(buffer, len);
        }

        static bool load_game_state_p1(unsigned char* buffer, int len)
        {
            return load_game_state(*g_game_state_p1, buffer, len);
        }

        static bool load_game_state_p2(unsigned char* buffer, int len)
        {
            return load_game_state(*g_game_state_p2, buffer, len);
        }

        static bool log_game_state(char* /*filename*/, unsigned char* /*buffer*/, int /*len*/)
        {
            return true;
        }

        static void free_buffer(void* buffer)
        {
            GameState::free_buffer(buffer);
        }

        static bool advance_frame(GameState& game)
        {
            int disconnect_flags = 0;
            input_t inputs[2];
            ggpo_synchronize_input(game.get_session(), &inputs, sizeof(inputs), &disconnect_flags);
            return game.advance_frame(inputs, disconnect_flags);
        }

        static bool advance_frame_p1(int)
        {
            return advance_frame(*g_game_state_p1);
        }

        static bool advance_frame_p2(int)
        {
            return advance_frame(*g_game_state_p2);
        }

        static bool on_event(GameState& game, GGPOEvent* event)
        {
            return game.on_event(event);
        }

        static bool on_event_p1(GGPOEvent* event)
        {
            return on_event(*g_game_state_p1, event);
        }

        static bool on_event_p2(GGPOEvent* event)
        {
            return on_event(*g_game_state_p2, event);
        }
    };
};

GameState* GameState::Callbacks::g_game_state_p1 = nullptr;
GameState* GameState::Callbacks::g_game_state_p2 = nullptr;

struct Settings
{
    int side;
    std::vector<input_t> input;
    uint16_t localport;
    const std::string remoteip = "127.0.0.1";
    uint16_t remoteport;
    int fps = 60;
};

void test_loop(const Settings& settings)
{
    GameState game(settings.side, settings.input, settings.localport, settings.remoteip, settings.remoteport);

    using std::chrono::high_resolution_clock;
    auto next = high_resolution_clock::now();
    for (;;)
    {
        if (game.input_ended())
            break;
        auto now = high_resolution_clock::now();
        const auto ms_left = std::chrono::duration_cast<std::chrono::milliseconds>(next - now).count();
        game.idle(std::max(0ll, ms_left - 1));
        if (now >= next)
        {
            game.run_frame();
            next = now + std::chrono::seconds(1) / settings.fps;
        }
    }
}

class GlobalInit
{
public:
    GlobalInit(UINT period) : m_period(period)
    {
        {
            auto result = ::timeBeginPeriod(m_period);
            assert(result == TIMERR_NOERROR);
            (void)result;
        }
        {
            WSADATA wd = { 0 };
            auto result = ::WSAStartup(MAKEWORD(2, 2), &wd);
            assert(result == 0);
            (void)result;
        }
    }
    ~GlobalInit()
    {
        ::timeEndPeriod(m_period);
        ::WSACleanup();
    }
private:
    UINT m_period;
};

int main()
{
    GlobalInit init(1);
    Settings settings_p1, settings_p2;

    settings_p1.side = 1;
    settings_p1.input = {{}};
    settings_p1.localport = 11111;
    settings_p1.remoteport = 22222;

    settings_p2.side = 2;
    settings_p2.input = {{}};
    settings_p2.localport = 22222;
    settings_p2.remoteport = 11111;

    std::thread threads[] = {
        std::thread(test_loop, settings_p1),
        std::thread(test_loop, settings_p2)
    };

    for (auto& t : threads)
        t.join();

    return 0;
}