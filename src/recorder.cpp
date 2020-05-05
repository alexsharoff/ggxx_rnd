
#include "recorder.h"

#include "util.h"

#include <algorithm>
#include <cwctype>
#include <fstream>
#include <sstream>
#include <string>
#include <time.h>
#include <utility>
#include <vector>


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

struct frame_advance_data
{
    struct saved_state
    {
        std::optional<game_state> state;
        IGame::input_t input;
    };
    int8_t speed = 1;
    std::deque<saved_state> state_history;
    std::optional<size_t> history_idx;
    uint16_t speed_control_counter = 0;
    bool out_of_memory = false;
    IGame::input_t prev_input;
    std::optional<bool> fps_limit;
    std::optional<bool> drawing_enabled;
    std::optional<time_t> fast_forward_start_time;
} g_frame_advance;

struct recorder_data
{
    std::vector<IGame::input_t> history;
    bool recording = false;
    bool playing = false;
    std::optional<uint32_t> initial_frame;
} g_recorder;

action g_prev_action{};
using key_map_t = std::unordered_map<uint8_t, action>;
key_map_t g_key_map;
configuration* g_cfg;

static_assert(sizeof(wchar_t) == sizeof(uint16_t));

struct replay_header
{
    char id[3] = { 'G', 'G', 'R' };
    uint8_t version[2] = { 0, 2 };
    uint8_t reserved[54] = { 0 };
    uint32_t body_size;
};

static_assert(sizeof(replay_header) == 64);

static_assert(sizeof(IGame::input_t) == 4);

struct replay_body_ver_01
{
    std::vector<IGame::input_t> input_history;
};

struct replay_body_ver_02
{
    struct memory_region
    {
        size_t rva;
        std::vector<char> data;
        bool dereference = false;
    };
    std::vector<memory_region> memory_regions;
    std::vector<IGame::input_t> input_history;
};

bool read_replay_body_ver_01(
    std::ifstream& ifs, const replay_header& header, replay_body_ver_01& body
)
{
    body.input_history.clear();
    constexpr auto input_data_size = 8;
    for (size_t i = 0; i < header.body_size / input_data_size; ++i)
    {
        IGame::input_t input{};
        if (!ifs.read(reinterpret_cast<char*>(&input), sizeof(input)))
            return false;
        uint32_t unused = 0;
        if (!ifs.read(reinterpret_cast<char*>(&unused), sizeof(unused)))
            return false;
        body.input_history.push_back(input);
    }
    return true;
}

bool read_replay_body_ver_02(
    std::ifstream& ifs, const replay_header&, replay_body_ver_02& body
)
{
    body.memory_regions.clear();
    body.input_history.clear();

    size_t memory_region_count = 0;
    if (!ifs.read(reinterpret_cast<char*>(&memory_region_count), sizeof(memory_region_count)))
        return false;

    size_t input_count = 0;
    if (!ifs.read(reinterpret_cast<char*>(&input_count), sizeof(input_count)))
        return false;

    while (memory_region_count--)
    {
        replay_body_ver_02::memory_region region{};
        if (!ifs.read(reinterpret_cast<char*>(&region.rva), sizeof(region.rva)))
            return false;
        uint8_t dereference = 0;
        if (!ifs.read(reinterpret_cast<char*>(&dereference), sizeof(dereference)))
            return false;
        region.dereference = dereference;
        size_t size = 0;
        if (!ifs.read(reinterpret_cast<char*>(&size), sizeof(size)))
            return false;
        region.data.resize(size);
        if (!ifs.read(&region.data[0], size))
            return false;

        body.memory_regions.push_back(region);
    }

    while (input_count--)
    {
        IGame::input_t input{};
        if (!ifs.read(reinterpret_cast<char*>(&input), sizeof(input)))
            return false;
        body.input_history.push_back(input);
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
        for (const auto& f : body.input_history)
            g_recorder.history.emplace_back(f);
    }
    else if (ver[0] == 0 && ver[1] == 2)
    {
        replay_body_ver_02 body;
        if (!read_replay_body_ver_02(ifs, header, body))
        {
            error = generic_error;
            return false;
        }
        g_recorder.history.clear();
        for (const auto& f : body.input_history)
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
    header.body_size = 2 * sizeof(size_t) + g_recorder.history.size() * sizeof(IGame::input_t);
    constexpr wchar_t generic_error[] = L"Write operation failed, replay may become corrupted.";
    if (!g_output_file.write(reinterpret_cast<const char*>(&header), sizeof(header)))
    {
        error = generic_error;
        return false;
    }

    size_t memory_regions_count = 0;
    if (!g_output_file.write(reinterpret_cast<const char*>(&memory_regions_count), sizeof(memory_regions_count)))
    {
        error = generic_error;
        return false;
    }

    size_t input_count = g_recorder.history.size();
    if (!g_output_file.write(reinterpret_cast<const char*>(&input_count), sizeof(input_count)))
    {
        error = generic_error;
        return false;
    }

    for (size_t i = 0; i < g_recorder.history.size(); ++i)
    {
        if (!g_output_file.write(reinterpret_cast<const char*>(&g_recorder.history[i]), sizeof(g_recorder.history[i])))
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
    if (!g_recorder.initial_frame.has_value())
        g_recorder.initial_frame = game->GetState().match2.frame.get();

    auto input = game->GetInputRemapped();

    // Replay logic:
    {
        if (g_recorder.playing)
        {
            const auto frame = game->GetState().match2.frame.get() - *g_recorder.initial_frame;
            if (frame < g_recorder.history.size())
                input = g_recorder.history[frame];
        }
    }

    // Manual frame advance logic:
    if (g_cfg->get_manual_frame_advance_settings().enabled)
    {
        action action{};
        if (::GetForegroundWindow() == game->GetWindowHandle())
        {
            for (const auto& [key, action_] : g_key_map)
            {
                if (::GetAsyncKeyState(key))
                    action |= action_;
            }
        }

        if (!(g_prev_action & action::frame_advance) && !!(action & action::frame_advance))
        {
            if (g_frame_advance.history_idx.has_value())
                g_frame_advance.history_idx = std::nullopt;
            else
                g_frame_advance.history_idx = g_frame_advance.state_history.size();
            g_frame_advance.speed = g_frame_advance.history_idx.has_value() ? 0 : 1;
            g_frame_advance.out_of_memory = false;
        }

        bool speed_control_enabled = false;
        if (g_frame_advance.history_idx.has_value())
        {
            const bool backward = !!(action & action::frame_prev);
            const bool forward = !!(action & action::frame_next);
            if (backward || forward)
            {
                speed_control_enabled = true;
                if (forward && (!(g_prev_action & action::frame_next) || g_frame_advance.speed_control_counter > 60))
                    g_frame_advance.speed = 1;
                else if (backward && (!(g_prev_action & action::frame_prev) || g_frame_advance.speed_control_counter > 60))
                    g_frame_advance.speed = -1;
                else
                    g_frame_advance.speed = 0;
                if (g_frame_advance.speed_control_counter > 0 || !(g_prev_action & action::frame_next))
                    ++g_frame_advance.speed_control_counter;
            }
        }
        else if (!!(action & action::frame_next))
        {
            if (!g_frame_advance.fps_limit.has_value())
            {
                g_frame_advance.fps_limit = game->IsFpsLimitEnabled();
                game->EnableFpsLimit(false);
            }
            if (!g_frame_advance.fast_forward_start_time.has_value())
            {
                g_frame_advance.fast_forward_start_time = ::time(0);
            }
            else if (::time(0) - *g_frame_advance.fast_forward_start_time > 2 && !g_frame_advance.drawing_enabled.has_value())
            {
                g_frame_advance.drawing_enabled = game->IsDrawingEnabled();
                game->EnableDrawing(false);
            }
        }

        if (g_frame_advance.fps_limit.has_value() && (g_frame_advance.drawing_enabled.has_value() || !(action & action::frame_next) || g_frame_advance.history_idx.has_value()))
        {
            game->EnableFpsLimit(*g_frame_advance.fps_limit);
            g_frame_advance.fps_limit = std::nullopt;
        }

        if (!(action & action::frame_next))
            g_frame_advance.fast_forward_start_time = std::nullopt;

        if (g_frame_advance.drawing_enabled.has_value() && (!(action & action::frame_next) || g_frame_advance.history_idx.has_value()))
        {
            game->EnableDrawing(*g_frame_advance.drawing_enabled);
            g_frame_advance.drawing_enabled = std::nullopt;
        }

        if (!speed_control_enabled)
        {
            g_frame_advance.speed = g_frame_advance.history_idx.has_value() ? 0 : 1;
            g_frame_advance.speed_control_counter = 0;
        }

        std::optional<decltype(input)> saved_input;
        if (g_frame_advance.history_idx.has_value() && g_frame_advance.history_idx < g_frame_advance.state_history.size())
        {
            auto& state_optional = g_frame_advance.state_history[*g_frame_advance.history_idx].state;
            if (state_optional.has_value())
                game->SetState(*state_optional);
            else
                state_optional = game->GetState();
            saved_input = g_frame_advance.state_history[*g_frame_advance.history_idx].input;
        }
        else
        {
            try
            {
                g_frame_advance.state_history.push_back({
                    game->GetState(),
                    input
                });
                size_t size_limit = 100;
                if (g_frame_advance.history_idx.has_value())
                    size_limit = 1500;
                while (g_frame_advance.state_history.size() > size_limit)
                    g_frame_advance.state_history.pop_front();
            }
            catch(const std::bad_alloc&)
            {
                g_frame_advance.out_of_memory = true;
                g_frame_advance.history_idx = std::nullopt;
            }
        }

        {
            const auto input_backup = input;
            for (size_t i = 0; i < 2; ++i)
            {
                const uint16_t bitmask = reverse_bytes(input[i]);
                if (g_frame_advance.history_idx.has_value())
                {
                    if (!!(action & action::frame_clear_input))
                    {
                        input[i] = 0;
                    }
                    else if (saved_input.has_value())
                    {
                        const auto saved_input_ = (*saved_input)[i];
                        const bool is_subset = (input[i] & saved_input_) == input[i];
                        if (input[i] == 0 || g_frame_advance.prev_input[i] != 0 && is_subset)
                        {
                            input[i] = saved_input_;
                        }
                    }
                }
            }
            g_frame_advance.prev_input = input_backup;
        }

        if (g_frame_advance.history_idx.has_value())
        {
            if (g_frame_advance.state_history[*g_frame_advance.history_idx].input != input)
            {
                g_frame_advance.state_history[*g_frame_advance.history_idx].input = input;
                // Input for current frame has changed. Invalidate saved state for next frames
                for (size_t i = *g_frame_advance.history_idx + 1; i < g_frame_advance.state_history.size(); ++i)
                    g_frame_advance.state_history[i].state = std::nullopt;
            }
            if (g_frame_advance.speed > 0)
                ++*g_frame_advance.history_idx;
            else if (g_frame_advance.speed < 0 && *g_frame_advance.history_idx > 0)
                --*g_frame_advance.history_idx;
        }

        g_prev_action = action;
    }

    // Replay logic:
    {
        if (g_recorder.playing)
        {
            const auto frame = game->GetState().match2.frame.get() - *g_recorder.initial_frame;
            if (frame >= g_recorder.history.size())
            {
                if (g_cfg->get_args().replay->mode == libgg_args::replay_t::mode_t::play)
                {
                    std::exit(0);
                }
                else
                {
                    //libgg_args::replay_t::mode_t::append

                    g_recorder.playing = false;
                    g_recorder.recording = true;
                    // stop at current frame to give a visual cue that replay has ended
                    g_frame_advance.history_idx = g_frame_advance.state_history.size();
                }
            }
        }
        if (g_recorder.recording)
        {
            const auto frame = game->GetState().match2.frame.get() - *g_recorder.initial_frame;
            if (g_recorder.history.size() <= frame)
                g_recorder.history.resize(frame + 1);
            g_recorder.history[frame] = input;
            std::wstring error;
            if (!update_replay_file(error))
            {
                std::wcerr << error.c_str() << std::endl;
            }
        }
    }

    if (g_frame_advance.history_idx.has_value() || g_recorder.playing)
        game->SetInputRemapped(input);

    return true;
}

bool process_objects_hook(IGame* game)
{
    match_state ms;
    if (g_frame_advance.history_idx.has_value())
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

        const auto frame = game->GetState().match2.frame.get();
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
    if (g_frame_advance.out_of_memory)
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
    if (args.replay.has_value())
    {
        const auto& path = args.replay->path;
        std::wstring error;
        if (args.replay->mode != libgg_args::replay_t::mode_t::record)
        {
            read_replay_file(path.c_str(), error);
            g_recorder.playing = true;
        }
        if (args.replay->mode != libgg_args::replay_t::mode_t::play)
        {
            open_replay_file(path.c_str(), error);
            g_recorder.recording = args.replay->mode != libgg_args::replay_t::mode_t::append;
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
