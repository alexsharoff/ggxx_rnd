
#include "recorder.h"

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

using action = keyboard_mapping::action;

constexpr action operator|(const action a, const action b)
{
    return static_cast<action>(static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
}
constexpr action operator&(const action a, const action b)
{
    return static_cast<action>(static_cast<uint16_t>(a) & static_cast<uint16_t>(b));
}
constexpr action& operator|=(action& a, const action b)
{
    a = a | b;
    return a;
}
constexpr action& operator&=(action& a, const action b)
{
    a = a & b;
    return a;
}
constexpr bool operator!(const action a)
{
    return static_cast<uint16_t>(a) == 0;
}

struct replay_frame
{
    IGame::input_t input;
    uint32_t state_checksum;
};
static_assert(sizeof(replay_frame) == 8);

struct frame_stop_data
{
    struct saved_state
    {
        std::optional<game_state> state;
        IGame::input_t input;
    };
    int8_t speed = 1;
    std::vector<saved_state> state_history;
    size_t history_idx = 0;
    bool enabled = false;
    uint16_t speed_control_counter = 0;
    bool out_of_memory = false;
    IGame::input_t prev_input;
    std::optional<bool> fps_limit;
} g_frame_stop;

struct recorder_data
{
    std::vector<replay_frame> history;
    bool recording = false;
    bool playing = false;
    std::optional<game_state> initial_state;
    size_t initial_frame = 0;
} g_recorder;

action g_prev_action{};
using key_map_t = std::unordered_map<uint8_t, action>;
key_map_t g_key_map;
configuration* g_cfg;

static_assert(sizeof(wchar_t) == sizeof(uint16_t));

struct replay_header
{
    char id[3] = { 'G', 'G', 'R' };
    uint8_t version[2] = { 0, 1 };
    skip_intro_settings skip_intro;
    uint8_t reserved[51] = { 0 };
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

    if (g_cfg->get_args().game_mode != libgg_args::game_mode_t::default)
    {
        // Allow overwriting game mode in replay using --gamemode argument
        // This may allow using the same replay for local and networking testing
        header.skip_intro.menu_idx = g_cfg->get_skip_intro_settings().menu_idx;
    }
    g_cfg->set_skip_intro_settings(header.skip_intro);

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
    header.skip_intro = g_cfg->get_skip_intro_settings();
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
    action action{};
    if (::GetForegroundWindow() == game->GetWindowHandle() && !g_cfg->get_args().noinput)
    {
        for (const auto& [key, action_] : g_key_map)
        {
            if (::GetAsyncKeyState(key))
                action |= action_;
        }
    }

    auto input = game->GetInputRemapped();

    // Frame stop logic:
    {
        if (!(g_prev_action & action::frame_pause) && !!(action & action::frame_pause))
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
            const bool backward = !!(action & action::backward);
            const bool forward = !!(action & action::forward);
            if (backward || forward)
            {
                speed_control_enabled = true;
                if (forward && (!(g_prev_action & action::forward) || g_frame_stop.speed_control_counter > 60))
                    g_frame_stop.speed = 1;
                else if (backward && (!(g_prev_action & action::backward) || g_frame_stop.speed_control_counter > 60))
                    g_frame_stop.speed = -1;
                else
                    g_frame_stop.speed = 0;
                ++g_frame_stop.speed_control_counter;
            }
        }
        else
        {
            if (!!(action & action::forward))
            {
                game->EnableFpsLimit(false);
                g_frame_stop.fps_limit = game->FpsLimitEnabled();
            }
            else if (g_frame_stop.fps_limit.has_value())
            {
                game->EnableFpsLimit(*g_frame_stop.fps_limit);
                g_frame_stop.fps_limit = std::nullopt;
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
                    if (!!(action & action::erase))
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
        if (g_cfg->get_args().replay_path.empty())
        {
            if (!(g_prev_action & action::memory_1) && !!(action & action::memory_1))
            {
                if (!!(action & action::erase))
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
                if (!g_cfg->get_args().replay_path.empty() && !g_cfg->get_args().replay_continue)
                    std::exit(0);
                else
                    g_recorder.playing = false;
            }
            else
            {
                if (g_cfg->get_args().replay_check)
                {
                    if (g_recorder.history[frame].state_checksum != state_checksum(game->GetState()))
                    {
                        std::cerr << "State checksum mismatch at frame " << frame << std::endl;
                        print_game_state(game->GetState());
                        std::exit(1);
                    }
                }
                input = g_recorder.history[frame].input;
            }
        }
        if (g_recorder.recording)
        {
            const auto frame = game->GetState().match2.clock.get() - g_recorder.initial_frame;
            if (g_recorder.history.size() <= frame)
                g_recorder.history.resize(frame + 1);
            g_recorder.history[frame].state_checksum = state_checksum(game->GetState());
            g_recorder.history[frame].input = input;
            if (!g_cfg->get_args().replay_path.empty())
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
            const auto& player_state = game->GetState().match.character_state.get()[0];
            const auto input = controller_state[0].bitmask_cur;
            game->DrawPressedDirection(input, 140, 300);
            game->DrawPressedButtons(input, player_state, 170, 300);
        }
        if (controller_state[1].bitmask_cur)
        {
            const auto& player_state = game->GetState().match.character_state.get()[1];
            const auto input = controller_state[1].bitmask_cur;
            game->DrawPressedDirection(input, 460, 300);
            game->DrawPressedButtons(input, player_state, 490, 300);
        }

        const auto frame = game->GetState().match2.clock.get();
        auto str = "FRAME " + std::to_string(frame);
        game->WriteCockpitFont(str.c_str(), 275, 440, 1, 0xFF, 1);
    }
    if (g_recorder.playing)
    {
        game->WriteCockpitFont("REPLAY", 285, 100, 1, 0xFF, 1);
    }
    else if (g_recorder.recording)
    {
        game->WriteCockpitFont("RECORDING", 270, 100, 1, 0xFF, 1);
    }
    else 
    if (g_frame_stop.out_of_memory)
    {
        game->WriteCockpitFont("OUT OF MEMORY!", 230, 440, 1, 0xff, 1);
    }

    return true;
}

key_map_t get_key_map(const keyboard_mapping& settings)
{
    key_map_t map;
    for (const auto [keycode, action] : settings.mapping)
        map[keycode] = action;
    return map;
}

}

void Initialize(IGame* game, configuration* cfg)
{
    g_cfg = cfg;
    g_key_map = get_key_map(cfg->get_keyboard_mapping());
    const auto& args = g_cfg->get_args();
    if (!args.replay_path.empty())
    {
        std::wstring error;
        if (args.replay_play)
        {
            read_replay_file(args.replay_path.c_str(), error);
            g_recorder.playing = true;
        }
        if (args.replay_record)
        {
            open_replay_file(args.replay_path.c_str(), error);
            g_recorder.recording = true;
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
