
#include "recorder.h"

#include "command_line.h"
#include "config.h"
#include "util.h"

#include <algorithm>
#include <cwctype>
#include <deque>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>


// TODO: record anywhere not just during training, but in menus etc
namespace recorder
{

namespace
{

using recorder_action = recorder_config::recorder_action;

constexpr recorder_action operator|(const recorder_action a, const recorder_action b)
{
    return static_cast<recorder_action>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
constexpr recorder_action operator&(const recorder_action a, const recorder_action b)
{
    return static_cast<recorder_action>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}
constexpr recorder_action& operator|=(recorder_action& a, const recorder_action b)
{
    a = a | b;
    return a;
}
constexpr recorder_action& operator&=(recorder_action& a, const recorder_action b)
{
    a = a & b;
    return a;
}
constexpr bool operator!(const recorder_action a)
{
    return static_cast<uint32_t>(a) == 0;
}

std::deque<game_state> g_state_history;
std::deque<std::array<uint16_t, 2>> g_input_history;
std::deque<uint32_t> g_checksum_history;
bool g_recording = false;
bool g_playing = false;
int8_t g_speed = 1;
bool g_manual_frame_advance = false;
uint16_t g_prev_bitmask[] = {0, 0};
size_t g_history_idx = 0;
uint16_t g_speed_control_counter = 0;
struct saved_state
{
    std::optional<game_state> state;
    std::optional<std::array<uint16_t, 2>> input;

    void reset()
    {
        state.reset();
        input.reset();
    }
};
saved_state g_saved_state;
bool g_out_of_memory = false;
recorder_action g_prev_action{};
recorder_config g_cfg;
command_line g_cmd;

static_assert(sizeof(wchar_t) == sizeof(uint16_t));

struct replay_header
{
    char id[3] = { 'G', 'G', 'R' };
    uint8_t version[2] = { 0, 1 };
    uint8_t reserved[55] = { 0 };
    uint32_t body_size;
};

static_assert(sizeof(replay_header) == 64);

struct replay_body_ver_01
{
    struct frame
    {
        uint16_t input[2];
        uint32_t state_checksum;
    };
    static_assert(sizeof(frame) == 8);
    std::vector<frame> frames;
};

bool read_replay_body_ver_01(
    std::ifstream& ifs, const replay_header& header, replay_body_ver_01& body
)
{
    body.frames.clear();
    constexpr auto frame_size = sizeof(replay_body_ver_01::frame);
    for (size_t i = 0; i < header.body_size / frame_size; ++i)
    {
        replay_body_ver_01::frame frame{};
        if (!ifs.read(reinterpret_cast<char*>(&frame), frame_size))
            return false;
        body.frames.push_back(frame);
    }
    return true;
}

bool read_input(const wchar_t* path, std::wstring& error)
{
    std::ifstream ifs(path, std::ifstream::binary);
    if (!ifs.is_open())
    {
        error = std::wstring(L"Unable to open file: ") + path;
        return false;
    }

    constexpr wchar_t generic_error[] = L"Replay is invalid or corrupted.";

    replay_header header{};
    if (!ifs.read(reinterpret_cast<char*>(&header), sizeof(header)))
    {
        error = generic_error;
        return false;
    }
    if (header.id[0] != 'G' || header.id[1] != 'G' || header.id[2] != 'R')
    {
        error = generic_error;
        return false;
    }

    const auto& ver = header.version;
    if (ver[0] == 0 && ver[1] == 1)
    {
        replay_body_ver_01 body;
        if (!read_replay_body_ver_01(ifs, header, body))
        {
            error = generic_error;
            return false;
        }
        g_input_history.clear();
        g_checksum_history.clear();
        for (const auto& f : body.frames)
        {
            g_checksum_history.push_back(f.state_checksum);
            g_input_history.push_back({f.input[0], f.input[1]});
        }
    }
    else
    {
        std::wostringstream oss;
        oss << L"Unsupported replay version: " << std::to_wstring(ver[0]) <<
               '.' << std::to_wstring(ver[1]) << '.';
        error = oss.str();
        return false;
    }

    return true;
}

std::ofstream g_output_file;
bool write_input(std::wstring& error)
{
    g_output_file.seekp(0);
    replay_header header{};
    constexpr auto frame_size = sizeof(replay_body_ver_01::frame);
    header.body_size = g_input_history.size() * frame_size;
    constexpr wchar_t generic_error[] = L"Write operation failed, replay may become corrupted.";
    if (!g_output_file.write(reinterpret_cast<const char*>(&header), sizeof(header)))
    {
        error = generic_error;
        return false;
    }

    for (size_t i = 0; i < g_input_history.size(); ++i)
    {
        const auto& input = g_input_history[i];
        replay_body_ver_01::frame frame{};
        frame.input[0] = input[0];
        frame.input[1] = input[1];
        frame.state_checksum = g_checksum_history[i];
        if (!g_output_file.write(reinterpret_cast<const char*>(&frame), frame_size))
        {
            error = generic_error;
            return false;
        }
    }
    return true;
}

bool open_output_file(const wchar_t* path, std::wstring& error)
{
    g_output_file.open(path, std::ofstream::trunc | std::ofstream::binary);
    if (!g_output_file.is_open())
    {
        error = std::wstring(L"Unable to open file: ") + path;
        return false;
    }
    return true;
}

// TODO: this function is kind of a mess, split/simplify
bool input_hook(IGame* game)
{
    if (!g_cmd.replay_path.empty())
    {
        if (g_cmd.replay_record)
        {
            const auto frame = game->GetState().match2.clock.get();
            if (g_input_history.size() <= frame)
            {
                g_input_history.resize(frame + 1);
                g_checksum_history.resize(frame + 1);
            }
            g_checksum_history[frame] = state_checksum(game->GetState());
            g_input_history[frame] = game->GetInput();
            std::wstring error;
            if (!write_input(error))
            {
                std::wcerr << error.c_str() << std::endl;
            }
        }
        else
        {
            const auto frame = game->GetState().match2.clock.get();
            if (frame >= g_input_history.size())
            {
                std::exit(0);
            }
            if (g_cmd.replay_check)
            {
                if (g_checksum_history[frame] != state_checksum(game->GetState()))
                {
                    std::cerr << "State checksum mismatch at frame " << frame << std::endl;
                    std::exit(1);
                }
            }
            game->SetInput(g_input_history[frame]);
        }
    }
    else if (game->InMatch() && game->InTrainingMode())
    {
        auto input = game->GetInput();

        if (g_saved_state.state.has_value())
            game->SetState(*g_saved_state.state);
        if (g_saved_state.input.has_value())
            game->SetInput(*g_saved_state.input);

        recorder_action action{};
        if (::GetForegroundWindow() == game->GetWindowHandle())
        {
            for (const auto& [key, action_] : g_cfg.key_map)
            {
                if (::GetAsyncKeyState(key))
                    action |= action_;
            }
        }

        const auto& controller_configs = game->GetGameConfig().player_controller_config;
        for (size_t i = 0; i < 2; ++i)
        {
            const uint16_t bitmask = reverse_bytes(input[i]);

            const auto& cfg = controller_configs[i];
            const uint16_t pause_bit = reverse_bytes(cfg.pause.bit);
            if (g_recording || g_playing || g_manual_frame_advance)
                input[i] = input[i] & ~pause_bit;

            if (g_manual_frame_advance)
            {
                const uint16_t reset_bit = reverse_bytes(cfg.reset.bit);
                input[i] = input[i] & ~reset_bit;

                if (bitmask & cfg.reset.bit)
                {
                    input[i] = 0;
                }
                else
                {
                    const uint16_t training_mode_buttons = reverse_bytes(
                        cfg.rec_player.bit | cfg.play_memory.bit | cfg.rec_enemy.bit |
                        cfg.enemy_jump.bit | cfg.enemy_walk.bit);
                    const uint16_t proposed_keys = game->GetInput()[i];
                    const uint16_t ignore_bits = training_mode_buttons | pause_bit | reset_bit;
                    const uint16_t prev_keys = reverse_bytes(g_prev_bitmask[i]) & ~ignore_bits;
                    const bool is_subset = (proposed_keys & input[i]) == input[i];
                    if (is_subset && (prev_keys != 0 || input[i] == 0))
                        input[i] = proposed_keys;
                }
            }

            g_prev_bitmask[i] = bitmask;
        }

        if (!(g_prev_action & recorder_action::memory_1) && !!(action & recorder_action::memory_1))
        {
            if (!!(action & recorder_action::erase))
            {
                g_input_history.clear();
                g_state_history.clear();
            }
            else
            {
                if (g_recording || g_playing)
                {
                    g_recording = false;
                    g_playing = false;
                }
                else
                {
                    if (g_input_history.empty())
                    {
                        g_playing = false;
                        g_recording = true;
                        g_out_of_memory = false;
                    }
                    else
                    {
                        g_recording = false;
                        g_playing = true;
                    }
                    g_history_idx = 0;
                }
            }
        }

        if (!(g_prev_action & recorder_action::frame_pause) && !!(action & recorder_action::frame_pause))
        {
            g_manual_frame_advance = !g_manual_frame_advance;
            g_speed = g_manual_frame_advance ? 0 : 1;
            g_out_of_memory = false;
        }

        bool speed_control_enabled = false;
        if (g_manual_frame_advance)
        {
            const bool backward = !!(action & recorder_action::backward);
            const bool forward = !!(action & recorder_action::forward);
            if (backward || forward)
            {
                speed_control_enabled = true;
                if (forward && (!(g_prev_action & recorder_action::forward) || g_speed_control_counter > 60))
                    g_speed = 1;
                else if (backward && (!(g_prev_action & recorder_action::backward) || g_speed_control_counter > 60))
                    g_speed = -1;
                else
                    g_speed = 0;
                ++g_speed_control_counter;
                g_prev_bitmask[0] = 0;
                g_prev_bitmask[1] = 0;
            }
        }
        else
        {
            if (!!(action & recorder_action::forward))
            {
                game->EnableFpsLimit(false);
            }
            else
            {
                game->EnableFpsLimit(true);
            }
        }

        g_prev_action = action;

        if (!speed_control_enabled)
        {
            g_speed = 0;
            g_speed_control_counter = 0;
        }

        if (g_playing)
        {
            if (g_history_idx < g_input_history.size())
            {
                if (g_history_idx == 0 && !g_state_history.empty())
                    game->SetState(g_state_history[g_history_idx]);

                game->SetInput(g_input_history[g_history_idx]);

                if (g_speed < 0)
                {
                    if (g_history_idx > 0)
                        --g_history_idx;
                }
                else if (g_speed > 0)
                {
                    ++g_history_idx;
                }
            }
            else
            {
                if (!g_manual_frame_advance)
                    g_playing = false;
            }

            if (g_history_idx >= g_state_history.size() && g_manual_frame_advance)
            {
                g_history_idx = g_state_history.size() - 1;
                g_speed = 0;
            }
        }
        else
        {
            if (g_recording)
            {
                const auto& input_ = game->GetInput();
                if (g_recording && (input[0] != input_[0] || input[1] != input_[1]))
                {
                    // recorded input at frame g_history_idx changed
                    // reset all recorded state after g_history_idx (but not input)
                    g_state_history.resize(g_history_idx);
                }
            }
            game->SetInput(input);
        }

        if (g_recording && g_history_idx < 5940)
        {
            try
            {
                if (g_history_idx >= g_state_history.size())
                    g_state_history.resize(g_history_idx + 1);
                if (g_history_idx >= g_input_history.size())
                    g_input_history.resize(g_history_idx + 1);
                g_state_history[g_history_idx] = game->GetState();
                g_input_history[g_history_idx] = game->GetInput();

                if (g_speed < 0)
                {
                    if (g_history_idx > 0)
                        --g_history_idx;
                }
                else if (g_speed > 0)
                {
                    ++g_history_idx;
                }
            }
            catch(const std::bad_alloc&)
            {
                g_out_of_memory = true;
                g_recording = false;
                g_history_idx = 0;
            }
        }
        else
        {
            g_recording = false;
        }

        g_saved_state.reset();
        if (g_manual_frame_advance)
        {
            if (g_recording || g_playing)
            {
                if (g_history_idx < g_state_history.size())
                    g_saved_state.state = g_state_history[g_history_idx];
                if (g_history_idx < g_input_history.size())
                    g_saved_state.input = g_input_history[g_history_idx];
            }
            else
            {
                if (g_speed <= 0)
                {
                    g_saved_state.state = game->GetState();
                    g_saved_state.input = game->GetInput();
                }
            }
        }
    }
    else
    {
        if (!game->GetState().match.pause_state.get())
        {
            g_state_history.clear();
            g_input_history.clear();
            g_recording = false;
            g_playing = false;
            g_speed = 1;
            g_manual_frame_advance = false;
            g_speed_control_counter = 0;
            g_out_of_memory = false;
            g_saved_state.reset();
        }
    }

    return true;
}

bool process_objects_hook(IGame* game)
{
    match_state ms;
    if (g_manual_frame_advance)
    {
        const auto& controller_state = game->GetState().match.controller_state.get();
        if (controller_state[0].bitmask_cur)
        {
            const auto input = controller_state[0].bitmask_cur;
            const auto obj = (active_object_state**)(game->GetImageBase() + 0x516778);
            game->DrawPressedDirection(input, 140, 300);
            game->DrawPressedButtons(input, *obj, 170, 300);
        }
        if (controller_state[1].bitmask_cur)
        {
            const auto input = controller_state[1].bitmask_cur;
            const auto obj = (active_object_state**)(game->GetImageBase() + 0x51A07C);
            game->DrawPressedDirection(input, 460, 300);
            game->DrawPressedButtons(input, *obj, 490, 300);
        }
    }
    game->WriteCockpitFont("DEV BUILD", 50, 100, 1, 0x50, 1);
    if (g_recording)
    {
        auto str = "REC " + std::to_string(g_history_idx);
        game->WriteCockpitFont(str.c_str(), 285, 100, 1, 0xFF, 1);
    }
    if (g_playing)
    {
        auto str = "PLAY " + std::to_string(g_history_idx);
        game->WriteCockpitFont(str.c_str(), 285, 100, 1, 0xFF, 1);
    }
    if (g_manual_frame_advance)
    {
        game->WriteCockpitFont("FRAME STOP", 260, 150, 1, 0xFF, 1);
    }
    if (g_out_of_memory)
    {
        game->WriteCockpitFont("OUT OF MEMORY!", 50, 150, 1, 0xff, 1);
    }

    return true;
}

}

void Initialize(IGame* game, recorder_config& cfg, const command_line& cmd)
{
    g_cfg = cfg;
    g_cmd = cmd;
    if (!g_cmd.replay_path.empty())
    {
        std::wstring error;
        if (g_cmd.replay_record)
            open_output_file(g_cmd.replay_path.c_str(), error);
        else
            read_input(g_cmd.replay_path.c_str(), error);

        if (!error.empty())
        {
            show_message_box(error.c_str(), true);
            std::exit(1);
        }
    }

    game->RegisterCallback(IGame::Event::AfterGetInput, input_hook);
    game->RegisterCallback(IGame::Event::AfterProcessObjects, process_objects_hook);
}

}
