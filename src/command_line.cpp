#include "command_line.h"

#include "util.h"

#include <Windows.h>
#include <shellapi.h>
#include <shlwapi.h>

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cwctype>
#include <filesystem>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>


namespace
{

std::vector<std::wstring> get_command_line()
{
    int num = 0;
    const auto str = ::CommandLineToArgvW(::GetCommandLineW(), &num);

    std::vector<std::wstring> result;
    for (int i = 1; i < num; ++i)
        result.push_back(str[i]);

    ::LocalFree(str);

    return result;
}

const wchar_t help_message[] = 
L"Supported arguments:\n"
"[/help]\n"
"[<path>.ggr [/record]]\n"
"[/unattended]\n"
"[/synctest <frame count> [/strict]]\n"
"[/gamemode {vs2p | training | network}]\n"
"[/printstate <frame> [<frame> ...]]"
"[/remoteip <ip or hostname>]\n"
"[/remoteport 7500]\n"
"[/localport 7500]\n"
"[/side {1 | 2}]\n"
;

void show_help(const wchar_t* reason = nullptr, bool is_error = false)
{
    std::wstring message;
    if (reason)
    {
        message = reason;
        message += L"\n\n";
    }
    message += help_message;
    show_message_box(message.c_str(), is_error);
}

std::wstring validate_path(const std::wstring& relpath, bool must_exist = false)
{
    if (must_exist && !std::filesystem::exists(relpath))
    {
        std::wostringstream oss;
        oss << L"Specified file does not exist:\n" << relpath;
        show_message_box(oss.str().c_str(), true);
        std::exit(1);
    }

    return std::filesystem::absolute(relpath).wstring();
}

std::wstring parse_replay_path(std::vector<std::wstring>& args, bool must_exist = false)
{
    std::wstring result;
    for (auto it = args.begin(); it != args.end(); ++it)
    {
        if (it->size() > 4 && it->at(0) != L'/')
        {
            auto path = *it;
            std::transform(path.begin(), path.end(), path.begin(), std::towlower);
            if (path.substr(path.size() - 4) == L".ggr")
            {
                result = validate_path(*it, must_exist);
                args.erase(it);
                break;
            }
        }
    }
    return result;
}

std::vector<std::wstring> parse_values(
    std::vector<std::wstring>& args, const std::wstring_view& flag, size_t size
)
{
    std::vector<std::wstring> result;
    const auto flat_it = std::find(args.begin(), args.end(), flag);
    if (flat_it == args.end())
        return result;
    if (size == std::numeric_limits<size_t>::max())
    {
        // unbound number of args
        // consume arguments until /flag is found
        auto next_flag_it = std::find_if(flat_it + 1, args.end(), [](const std::wstring& str)
        {
            return str.size() >= 2 && str.substr(0, 2) == L"/";
        });
        size = std::distance(flat_it, next_flag_it) - 1;
    }
    const size_t remaining_args = args.size() - std::distance(args.begin(), flat_it);
    if (flat_it != args.end() && size < remaining_args)
    {
        const auto end = flat_it + size + 1;
        result.assign(flat_it, end);
        args.erase(flat_it, end);
    }
    return result;
}

bool parse_option(std::vector<std::wstring>& args, const std::wstring_view& flag)
{
    return !parse_values(args, flag, 0).empty();
}

std::wstring parse_path(
    std::vector<std::wstring>& args, const std::wstring_view& flag, bool must_exist = false
)
{
    const auto result = parse_values(args, flag, 1);
    if (result.empty())
        return std::wstring();

    return validate_path(result[1], must_exist);
}

std::vector<int> parse_integers(
    std::vector<std::wstring>& args, const std::wstring_view& flag, size_t size
)
{
    assert(size > 0);

    std::vector<int> integers;
    const auto strings = parse_values(args, flag, size);
    if (strings.empty())
        return integers;

    for (auto it = strings.begin() + 1; it != strings.end(); ++it)
    {
        try
        {
            integers.push_back(std::stoi(*it));
        }
        catch(const std::exception& e)
        {
            std::wostringstream oss;
            oss << L"Invalid " << flag << L" value " << *it << ": " << e.what();
            show_message_box(oss.str().c_str(), true);
            std::exit(1);
        }
    }

    return integers;
}

std::optional<uint16_t> parse_port(std::vector<std::wstring>& args, const std::wstring_view& flag)
{
    auto integers = parse_integers(args, flag, 1);
    if (!integers.empty())
    {
        if (integers[0] == 0 || integers[0] > 65535)
        {
            show_message_box(L"Invalid port numer", true);
            std::exit(1);
        }
        return static_cast<uint16_t>(integers[0]);
    }
    return std::nullopt;
}

std::optional<libgg_args::game_mode_t>
parse_game_mode(std::vector<std::wstring>& args, const std::wstring_view& flag)
{
    const auto result = parse_values(args, flag, 1);
    if (result.empty())
        return std::nullopt;

    const auto& str = result[1];
    if (str == L"vs2p")
    {
        return libgg_args::game_mode_t::vs2p;
    }
    else if (str == L"network")
    {
        return libgg_args::game_mode_t::network;
    }
    else if (str == L"training")
    {
        return libgg_args::game_mode_t::training;
    }
    else
    {
        std::wostringstream oss;
        oss << L"Invalid " << flag << L" value: '" << str << "'";
        show_message_box(oss.str().c_str(), true);
        std::exit(1);
    }
}

}

libgg_args parse_command_line()
{
    libgg_args cmd;

    auto args = get_command_line();
    if (!args.empty())
    {
        if (parse_option(args, L"/help"))
        {
            show_help();
            std::exit(0);
        }

        bool record = parse_option(args, L"/record");
        auto path = parse_replay_path(args, !record);
        if (!path.empty())
        {
            auto mode = libgg_args::replay_t::mode_t::play;
            if (record)
            {
                if (std::filesystem::exists(path))
                    mode = libgg_args::replay_t::mode_t::append;
                else
                    mode = libgg_args::replay_t::mode_t::record;
            }
            cmd.replay = { path, mode };
        }

        auto port_opt = parse_port(args, L"/localport");
        if (port_opt.has_value())
            cmd.network.localport = *port_opt;

        port_opt = parse_port(args, L"/remoteport");
        if (port_opt.has_value())
            cmd.network.remoteport = *port_opt;

        auto integers = parse_integers(args, L"/synctest", 1);
        if (!integers.empty())
        {
            cmd.synctest_frames = integers[0];
            if (cmd.synctest_frames < 1)
            {
                show_message_box(L"Invalid /synctest value", true);
                std::exit(1);
            }
            cmd.synctest_strict = parse_option(args, L"/strict");
        }

        integers = parse_integers(args, L"/side", 1);
        if (!integers.empty())
        {
            cmd.network.side = static_cast<uint8_t>(integers[0]);
            if (cmd.network.side < 1 || cmd.network.side > 2)
            {
                show_message_box(L"Invalid /side value", true);
                std::exit(1);
            }
        }

        auto values = parse_values(args, L"/remoteip", 1);
        if (!values.empty())
        {
            cmd.network.remoteip.clear();
            for (auto wc : values[1])
                cmd.network.remoteip.push_back(static_cast<char>(wc));
        }

        cmd.printstate = parse_integers(args, L"/printstate", std::numeric_limits<size_t>::max());
        cmd.game_mode = parse_game_mode(args, L"/gamemode");
        cmd.unattended = parse_option(args, L"/unattended");

        if (!args.empty())
        {
            std::wostringstream oss;
            std::copy(args.begin(), args.end(), std::ostream_iterator<std::wstring, wchar_t>(oss, L" "));
            auto args_str = oss.str();
            // remove trailing space
            args_str = args_str.substr(0, oss.str().size() - 1);
            const auto message = L"Unknown arguments:\n" + args_str;
            show_help(message.c_str(), true);
            std::exit(1);
        }
    }

    return cmd;
}
