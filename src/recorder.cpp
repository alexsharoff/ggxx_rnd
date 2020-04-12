
#include "recorder.h"

#include "command_line.h"
#include "config.h"
#include "util.h"

#include <algorithm>
#include <cwctype>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>


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

struct replay_frame
{
    std::array<uint16_t, 2> input;
    uint32_t state_checksum;
};
static_assert(sizeof(replay_frame) == 8);

struct frame_stop_data
{
    struct saved_state
    {
        std::optional<game_state> state;
        std::array<uint16_t, 2> input;
    };
    int8_t speed = 1;
    std::vector<saved_state> state_history;
    size_t history_idx = 0;
    bool enabled = false;
    uint16_t speed_control_counter = 0;
    bool out_of_memory = false;
    std::array<uint16_t, 2> prev_input;
} g_frame_stop;

struct recorder_data
{
    std::vector<replay_frame> history;
    bool recording = false;
    bool playing = false;
    std::optional<game_state> initial_state;
    size_t initial_frame = 0;
} g_recorder;

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
    std::vector<replay_frame> frames;
};

bool read_replay_body_ver_01(
    std::ifstream& ifs, const replay_header& header, replay_body_ver_01& body
)
{
    body.frames.clear();
    constexpr auto frame_size = sizeof(replay_frame);
    for (size_t i = 0; i < header.body_size / frame_size; ++i)
    {
        replay_frame frame{};
        if (!ifs.read(reinterpret_cast<char*>(&frame), frame_size))
            return false;
        body.frames.push_back(frame);
    }
    return true;
}

bool read_replay_file(const wchar_t* path, std::wstring& error)
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
        g_recorder.history.clear();
        for (const auto& f : body.frames)
            g_recorder.history.emplace_back(f);
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
// call before update_replay_file
bool open_replay_file(const wchar_t* path, std::wstring& error)
{
    g_output_file.open(path, std::ofstream::trunc | std::ofstream::binary);
    if (!g_output_file.is_open())
    {
        error = std::wstring(L"Unable to open file: ") + path;
        return false;
    }
    return true;
}

bool update_replay_file(std::wstring& error)
{
    g_output_file.seekp(0);
    replay_header header{};
    constexpr auto frame_size = sizeof(replay_frame);
    header.body_size = g_recorder.history.size() * frame_size;
    constexpr wchar_t generic_error[] = L"Write operation failed, replay may become corrupted.";
    if (!g_output_file.write(reinterpret_cast<const char*>(&header), sizeof(header)))
    {
        error = generic_error;
        return false;
    }

    for (size_t i = 0; i < g_recorder.history.size(); ++i)
    {
        if (!g_output_file.write(reinterpret_cast<const char*>(&g_recorder.history[i]), frame_size))
        {
            error = generic_error;
            return false;
        }
    }
    return true;
}

// TODO: this function is kind of a mess, split/simplify
bool input_hook(IGame* game)
{
    recorder_action action{};
    if (::GetForegroundWindow() == game->GetWindowHandle())
    {
        for (const auto& [key, action_] : g_cfg.key_map)
        {
            if (::GetAsyncKeyState(key))
                action |= action_;
        }
    }

    auto input = game->GetInputRemapped();

    // Frame stop logic:
    {
        if (!(g_prev_action & recorder_action::frame_pause) && !!(action & recorder_action::frame_pause))
        {
            g_frame_stop.enabled = !g_frame_stop.enabled;
            g_frame_stop.speed = g_frame_stop.enabled ? 0 : 1;
            g_frame_stop.out_of_memory = false;
            if (g_frame_stop.enabled)
            {
                g_frame_stop.history_idx = 0;
                g_frame_stop.state_history.clear();
            }
        }

        bool speed_control_enabled = false;
        if (g_frame_stop.enabled)
        {
            const bool backward = !!(action & recorder_action::backward);
            const bool forward = !!(action & recorder_action::forward);
            if (backward || forward)
            {
                speed_control_enabled = true;
                if (forward && (!(g_prev_action & recorder_action::forward) || g_frame_stop.speed_control_counter > 60))
                    g_frame_stop.speed = 1;
                else if (backward && (!(g_prev_action & recorder_action::backward) || g_frame_stop.speed_control_counter > 60))
                    g_frame_stop.speed = -1;
                else
                    g_frame_stop.speed = 0;
                ++g_frame_stop.speed_control_counter;
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

        if (!speed_control_enabled)
        {
            g_frame_stop.speed = g_frame_stop.enabled ? 0 : 1;
            g_frame_stop.speed_control_counter = 0;
        }

        std::optional<decltype(input)> saved_input;

        if (g_frame_stop.enabled)
        {
            if (g_frame_stop.history_idx < g_frame_stop.state_history.size())
            {
                auto& state_optional = g_frame_stop.state_history[g_frame_stop.history_idx].state;
                if (state_optional.has_value())
                    game->SetState(*state_optional);
                else
                    state_optional = game->GetState();
                saved_input = g_frame_stop.state_history[g_frame_stop.history_idx].input;
            }
            else
            {
                try
                {
                    g_frame_stop.state_history.push_back({
                        game->GetState(),
                        input
                    });
                }
                catch(const std::bad_alloc&)
                {
                    g_frame_stop.out_of_memory = true;
                    g_frame_stop.enabled = false;
                }
            }
        }

        {
            const auto input_backup = input;
            for (size_t i = 0; i < 2; ++i)
            {
                const uint16_t bitmask = reverse_bytes(input[i]);
                if (g_frame_stop.enabled)
                {
                    if (!!(action & recorder_action::erase))
                    {
                        input[i] = 0;
                    }
                    else if (saved_input.has_value())
                    {
                        const auto saved_input_ = (*saved_input)[i];
                        const bool is_subset = (input[i] & saved_input_) == input[i];
                        if (input[i] == 0 || g_frame_stop.prev_input[i] != 0 && is_subset)
                        {
                            input[i] = saved_input_;
                        }
                    }
                }
            }
            g_frame_stop.prev_input = input_backup;
        }

        if (g_frame_stop.enabled)
        {
            if (g_frame_stop.state_history[g_frame_stop.history_idx].input != input)
            {
                g_frame_stop.state_history[g_frame_stop.history_idx].input = input;
                // Input for current frame has changed. Invalidate saved state for next frames
                for (size_t i = g_frame_stop.history_idx + 1; i < g_frame_stop.state_history.size(); ++i)
                    g_frame_stop.state_history[i].state = std::nullopt;
            }
            if (g_frame_stop.speed > 0)
                ++g_frame_stop.history_idx;
            else if (g_frame_stop.speed < 0 && g_frame_stop.history_idx > 0)
                --g_frame_stop.history_idx;
        }
    }

    // Replay logic:
    {
        // Disable manual replay control if replay was provided via command line
        if (g_cmd.replay_path.empty())
        {
            if (!(g_prev_action & recorder_action::memory_1) && !!(action & recorder_action::memory_1))
            {
                if (!!(action & recorder_action::erase))
                {
                    g_recorder.history.clear();
                }
                else
                {
                    if (g_recorder.recording || g_recorder.playing)
                    {
                        g_recorder.recording = false;
                        g_recorder.playing = false;
                    }
                    else
                    {
                        if (g_recorder.history.empty())
                        {
                            g_recorder.playing = false;
                            g_recorder.recording = true;
                            const auto& state = game->GetState();
                            g_recorder.initial_frame = state.match2.clock.get();
                            g_recorder.initial_state = state;
                        }
                        else
                        {
                            g_recorder.playing = true;
                            g_recorder.recording = false;
                            game->SetState(*g_recorder.initial_state);
                            g_frame_stop.state_history.clear();
                            g_frame_stop.history_idx = 0;
                        }
                    }
                }
            }
        }

        if (g_recorder.playing)
        {
            const auto frame = game->GetState().match2.clock.get() - g_recorder.initial_frame;
            if (frame >= g_recorder.history.size())
            {
                if (!g_cmd.replay_path.empty())
                    std::exit(0);
                else
                    g_recorder.playing = false;
            }
            else
            {
                if (g_cmd.replay_check)
                {
                    if (g_recorder.history[frame].state_checksum != state_checksum(game->GetState()))
                    {
                        std::cerr << "State checksum mismatch at frame " << frame << std::endl;
                        std::exit(1);
                    }
                }
                input = g_recorder.history[frame].input;
            }
        }
        else if (g_recorder.recording)
        {
            const auto frame = game->GetState().match2.clock.get() - g_recorder.initial_frame;
            if (g_recorder.history.size() <= frame)
                g_recorder.history.resize(frame + 1);
            g_recorder.history[frame].state_checksum = state_checksum(game->GetState());
            g_recorder.history[frame].input = input;
            if (!g_cmd.replay_path.empty())
            {
                std::wstring error;
                if (!update_replay_file(error))
                {
                    std::wcerr << error.c_str() << std::endl;
                }
            }
        }
    }

    g_prev_action = action;
    if (g_frame_stop.enabled || g_recorder.playing)
        game->SetInputRemapped(input);

    return true;
}

bool process_objects_hook(IGame* game)
{
    match_state ms;
    if (g_frame_stop.enabled)
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

        const auto frame = game->GetState().match2.clock.get();
        auto str = "FRAME " + std::to_string(frame);
        game->WriteCockpitFont(str.c_str(), 275, 440, 1, 0xFF, 1);
    }
    if (g_recorder.recording)
    {
        game->WriteCockpitFont("REPLAY REC", 270, 100, 1, 0xFF, 1);
    }
    if (g_recorder.playing)
    {
        game->WriteCockpitFont("REPLAY", 285, 100, 1, 0xFF, 1);
    }
    if (g_frame_stop.out_of_memory)
    {
        game->WriteCockpitFont("OUT OF MEMORY!", 230, 440, 1, 0xff, 1);
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
        {
            open_replay_file(g_cmd.replay_path.c_str(), error);
            g_recorder.recording = true;
        }
        else
        {
            read_replay_file(g_cmd.replay_path.c_str(), error);
            g_recorder.playing = true;
        }

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
