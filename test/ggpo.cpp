#include <ggponet.h>

#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

#include <Windows.h>
#include <timeapi.h>
#include <Winsock2.h>

#include "test.h"


#define GGPO_CHECK(expr) \
    do { \
        const auto res___ = expr; \
        if (!GGPO_SUCCEEDED(res___)) { \
            std::cerr << __FILE__ << ':' << __LINE__ << ": " << #expr << " == " << res___ << std::endl; \
            throw std::logic_error("GGPO error"); \
        } \
    } while(false)


using input_t = std::array<int8_t, 4>;
using inputs_vector_t = std::vector<std::array<input_t, 2>>;

class TestGame
{
public:
    TestGame(int side, const inputs_vector_t& inputs, uint16_t localport, const std::string_view& remoteip, uint16_t remoteport)
    {
        GGPOSessionCallbacks callbacks;
        if (side == 2)
        {
            callbacks.advance_frame = Callbacks::advance_frame_p2;
            callbacks.on_event = Callbacks::on_event_p2;
            callbacks.load_game_state = Callbacks::load_game_state_p2;
            callbacks.save_game_state = Callbacks::save_game_state_p2;
            Callbacks::g_game_state_p2 = this;
        }
        else
        {
            callbacks.advance_frame = Callbacks::advance_frame_p1;
            callbacks.on_event = Callbacks::on_event_p1;
            callbacks.load_game_state = Callbacks::load_game_state_p1;
            callbacks.save_game_state = Callbacks::save_game_state_p1;
            Callbacks::g_game_state_p1 = this;
        }
        callbacks.free_buffer = Callbacks::free_buffer;
        callbacks.begin_game = Callbacks::begin_game;
        callbacks.log_game_state = Callbacks::log_game_state;
        GGPO_CHECK(ggpo_start_session(&m_session, &callbacks, "TEST", 2, 4, localport));

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

        m_frame_inputs = inputs;
        m_side = side;
    }

    ~TestGame()
    {
        ggpo_close_session(m_session);
    }

    bool advance_frame(const std::array<input_t, 2>& inputs, int /* disconnect_flags */)
    {
        for (const auto& input : inputs)
        {
            for (int i = 0; i < 4; ++i)
            {
                if (input[i] < 0)
                    --m_state.text[i];
                if (input[i] > 0)
                    ++m_state.text[i];
            }
        }
        ++m_state.frame;
        ggpo_advance_frame(m_session);
        return true;
    }

    bool save_game_state(unsigned char** buffer, int* len , int* checksum, int frame) const
    {
        (void)frame;
        assert(static_cast<size_t>(frame) == m_state.frame);
        *checksum = *reinterpret_cast<const int*>(m_state.text);
        *buffer = new unsigned char[sizeof(m_state)];
        auto data = reinterpret_cast<const unsigned char*>(&m_state);
        std::copy(data, data + sizeof(m_state), *buffer);
        *len = sizeof(m_state);
        return true;
    }

    static void free_buffer(void* buffer)
    {
        auto p = (char*)buffer;
        delete[] buffer;
        std::memset(p, -1, 12);
    }

    bool load_game_state(unsigned char* buffer, int len)
    {
        (void)len;
        assert(sizeof(m_state) == len);
        auto data = reinterpret_cast<unsigned char*>(&m_state);
        std::copy(buffer, buffer + sizeof(m_state), data);
        return true;
    }

    bool on_event(GGPOEvent* event)
    {
        std::lock_guard<std::mutex> guard(s_stdout_mutex);
        print_log_prefix();
        int sync_progress = 0;
        switch (event->code)
        {
        case GGPO_EVENTCODE_CONNECTED_TO_PEER:
            std::cout << "Connected" << std::endl;
            break;
        case GGPO_EVENTCODE_SYNCHRONIZING_WITH_PEER:
            sync_progress = event->u.synchronizing.count * 100 / event->u.synchronizing.total;
            std::cout << "Synchronizing: " << sync_progress << '%' << std::endl;
            break;
        case GGPO_EVENTCODE_SYNCHRONIZED_WITH_PEER:
            std::cout << "Synchronizing: done" << std::endl;
            break;
        case GGPO_EVENTCODE_RUNNING:
            std::cout << "Running" << std::endl;
            break;
        case GGPO_EVENTCODE_CONNECTION_INTERRUPTED:
            std::cout << "Connection interrupted" << std::endl;
            break;
        case GGPO_EVENTCODE_CONNECTION_RESUMED:
            std::cout << "Connection resumed" << std::endl;
            break;
        case GGPO_EVENTCODE_DISCONNECTED_FROM_PEER:
            std::cout << "Disconnected" << std::endl;
            break;
        case GGPO_EVENTCODE_TIMESYNC:
            std::cout << "Timesync: " << event->u.timesync.frames_ahead << " frames ahead" << std::endl;
            m_frames_ahead = event->u.timesync.frames_ahead;
            break;
        default:
            assert(0);
        }
        return true;
    }

    bool input_ended() const
    {
        return m_frame_inputs.size() <= m_state.frame;
    }

    void idle(uint64_t timeout_ms) const
    {
        ggpo_idle(m_session, static_cast<int>(timeout_ms));
    }

    void run_frame()
    {
        if (!m_frames_ahead)
        {
            GGPOErrorCode result = GGPO_OK;
            auto frame_inputs = m_frame_inputs[m_state.frame];
            for (int side = 1; side <= 2; ++side)
            {
                if (m_side == 0 || m_side == side)
                {
                    auto& input = frame_inputs[side-1];
                    result = ggpo_add_local_input(
                        m_session, m_player_handles[side-1], &input, sizeof(input)
                    );
                }
            }
            if (GGPO_SUCCEEDED(result))
            {
                int disconnect_flags;
                std::array<input_t, 2> inputs;
                result = ggpo_synchronize_input(m_session, &inputs, sizeof(inputs), &disconnect_flags);
                if (GGPO_SUCCEEDED(result))
                {
                    advance_frame(inputs, disconnect_flags);
                }
            }
        }
        else
        {
            std::lock_guard<std::mutex> guard(s_stdout_mutex);
            print_log_prefix();
            std::cout << "Timesync" << std::endl;
            --m_frames_ahead;
        }
        draw_current_frame();
    }

    void draw_current_frame() const
    {
        std::lock_guard<std::mutex> guard(s_stdout_mutex);
        print_log_prefix();
        std::cout << m_state.text << std::endl;
    }

    void print_log_prefix() const
    {
        std::cout << 'f' << m_state.frame << " s" << m_side << ": ";
    }

    struct GameState
    {
        size_t frame = 0;
        char text[5] = "TEST";
    } m_state;

    const GameState& state() const
    {
        return m_state;
    }

private:
    static std::mutex s_stdout_mutex;

    GGPOSession* m_session;
    GGPOPlayerHandle m_player_handles[2];
    int m_side;
    int m_frames_ahead = 0;
    inputs_vector_t m_frame_inputs;

    GGPOSession* get_session() const
    {
        return m_session;
    }

    struct Callbacks
    {
        static TestGame* g_game_state_p1;
        static TestGame* g_game_state_p2;
        static bool begin_game(const char*)
        {
            return true;
        }

        static bool save_game_state(TestGame& game, unsigned char** buffer, int* len, int* checksum, int frame)
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

        static bool load_game_state(TestGame& game, unsigned char* buffer, int len)
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
            TestGame::free_buffer(buffer);
        }

        static bool advance_frame(TestGame& game)
        {
            int disconnect_flags = 0;
            std::array<input_t, 2> inputs;
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

        static bool on_event(TestGame& game, GGPOEvent* event)
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

TestGame* TestGame::Callbacks::g_game_state_p1 = nullptr;
TestGame* TestGame::Callbacks::g_game_state_p2 = nullptr;
std::mutex TestGame::s_stdout_mutex;

void game_loop(TestGame& game, uint32_t fps)
{
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
            next = now + std::chrono::seconds(1) / fps;
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

    inputs_vector_t inputs(60);
    inputs[28][0][0] = -1;
    inputs[28][1][0] = 1;
    inputs[28][1][1] = 1;
    uint16_t port_p1 = 11111;
    uint16_t port_p2 = 22222;
    TestGame game_p1(1, inputs, port_p1, "127.0.0.1", port_p2);
    TestGame game_p2(2, inputs, port_p2, "127.0.0.1", port_p1);

    std::thread threads[] = {
        std::thread(game_loop, std::ref(game_p1), 60),
        std::thread(game_loop, std::ref(game_p2), 60)
    };

    for (auto& t : threads)
        t.join();

    TEST_EQ(std::string_view(game_p1.state().text), game_p2.state().text);

    return 0;
}
