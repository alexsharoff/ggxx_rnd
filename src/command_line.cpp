#include "command_line.h"

#include "util.h"

#include <Windows.h>
#include <shellapi.h>
#include <shlwapi.h>

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cwctype>
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
"--help: show this message\n"
"[{--replay <path> | <path>.ggr} [--checkstate] [--continue] [--rollback <from frame> <to frame> [--rollback ...]]\n"
"[--record <path>]\n"
"[--nographics]\n"
"[--nosound]\n"
"[--noinput]\n"
"[--synctest <frame count>]\n"
"[--displaycfg <path>]\n"
"[--savedata <path>]\n"
"[--libggcfg <path>]\n"
"[--usedefaults]\n"
"[--gamemode {vs2p | training | network}]\n"
"[--printstate <frame> [<frame> ...]]"
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

std::wstring parse_replay_path(std::vector<std::wstring>& args)
{
    std::wstring result;
    if (!args.empty() && args[0].size() > 4)
    {
        auto path = args[0];
        std::transform(path.begin(), path.end(), path.begin(), std::towlower);
        if (path.substr(path.size() - 4) == L".ggr")
        {
            result = args[0];
            args.erase(args.begin());
        }
    }
    return result;
}

std::vector<std::wstring> parse_values(
    std::vector<std::wstring>& args, const std::wstring& flag, size_t size
)
{
    std::vector<std::wstring> result;
    const auto flat_it = std::find(args.begin(), args.end(), flag);
    if (flat_it == args.end())
        return result;
    if (size == std::numeric_limits<size_t>::max())
    {
        // unbound number of args
        // consume arguments until --flag is found
        auto next_flag_it = std::find_if(flat_it + 1, args.end(), [](const std::wstring& str)
        {
            return str.size() >= 2 && str.substr(0, 2) == L"--";
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

bool parse_option(std::vector<std::wstring>& args, const std::wstring& flag)
{
    return !parse_values(args, flag, 0).empty();
}

std::wstring parse_path(
    std::vector<std::wstring>& args, const std::wstring& flag, bool must_exist = false
)
{
    const auto result = parse_values(args, flag, 1);
    if (result.empty())
        return std::wstring();

    const auto& relpath = result[1];

    if (must_exist)
    {
        if (!::PathFileExistsW(relpath.c_str()))
        {
            std::wostringstream oss;
            oss << L"Specified path does not exist:\n" << relpath;
            show_message_box(oss.str().c_str(), true);
            std::exit(1);
        }
    }

    wchar_t buffer[MAX_PATH + 1];
    ::GetFullPathNameW(relpath.c_str(), MAX_PATH + 1, buffer, NULL);

    return buffer;
}

std::vector<int> parse_integers(
    std::vector<std::wstring>& args, const std::wstring& flag, size_t size
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

libgg_args::game_mode_t parse_game_mode(
    std::vector<std::wstring>& args, const std::wstring& flag
)
{
    const auto result = parse_values(args, flag, 1);
    if (result.empty())
        return libgg_args::game_mode_t::default;

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
        if (parse_option(args, L"--help"))
        {
            show_help();
            std::exit(0);
        }

        auto path = parse_replay_path(args);
        if (!path.empty())
        {
            cmd.replay_path = path;
            cmd.replay_play = true;
        }

        path = parse_path(args, L"--replay", true);
        if (!path.empty())
        {
            cmd.replay_path = path;
            cmd.replay_play = true;
            if (parse_option(args, L"--record"))
            {
                // update replay (overdub)
                cmd.replay_record = true;
            }
        }

        path = parse_path(args, L"--record");
        if (!path.empty())
        {
            cmd.replay_path = path;
            cmd.replay_record = true;
        }

        auto integers = parse_integers(args, L"--synctest", 1);
        if (!integers.empty())
            cmd.synctest_frames = integers[0];

        for (;;)
        {
            integers = parse_integers(args, L"--rollback", 2);
            if (integers.empty())
                break;
            cmd.rollback_map[integers[0]].push_back(integers[1]);
        }

        cmd.printstate = parse_integers(args, L"--printstate", std::numeric_limits<size_t>::max());
        cmd.replay_check = parse_option(args, L"--checkstate");
        cmd.replay_continue = parse_option(args, L"--continue");
        cmd.nographics = parse_option(args, L"--nographics");
        cmd.nosound = parse_option(args, L"--nosound");
        cmd.noinput = parse_option(args, L"--noinput");
        cmd.game_mode = parse_game_mode(args, L"--gamemode");
        cmd.usedefaults = parse_option(args, L"--usedefaults");
        cmd.displaycfg = parse_path(args, L"--displaycfg");
        cmd.savedata = parse_path(args, L"--savedata");
        cmd.libggcfg = parse_path(args, L"--libggcfg");

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
