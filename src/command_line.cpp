#include "command_line.h"

#include <Windows.h>
#include <shellapi.h>

#include <algorithm>
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
"<replay file path>.ggr\n"
"--record <path>\n"
"--replay <path>\n"
;

void show_help(const wchar_t* reason = nullptr, bool is_error = false)
{
    UINT type = is_error ? MB_ICONWARNING : MB_ICONINFORMATION;
    type |= MB_TASKMODAL | MB_TOPMOST;
    std::wstring message;
    if (reason)
    {
        message = reason;
        message += L"\n\n";
    }
    message += help_message;
    ::MessageBoxW(NULL, message.c_str(), L"GGXXACPR", type);
}

}

command_line parse_command_line()
{
    command_line cmd;

    auto args = get_command_line();
    if (!args.empty())
    {
        auto found = std::find(args.begin(), args.end(), L"--help");
        if (found != args.end())
        {
            show_help();
            std::exit(0);
        }

        if (!args.empty() && args[0].size() > 4)
        {
            auto path = args[0];
            std::transform(path.begin(), path.end(), path.begin(), std::towlower);
            if (path.substr(path.size() - 4) == L".ggr")
            {
                cmd.replay_path = args[0];
                cmd.replay = true;
                args.erase(args.begin());
            }
        }

        found = std::find(args.begin(), args.end(), L"--record");
        if (found != args.end() && (found + 1) != args.end())
        {
            cmd.replay_path = *(found + 1);
            cmd.record = true;
            args.erase(found, found + 2);
        }

        found = std::find(args.begin(), args.end(), L"--replay");
        if (found != args.end() && (found + 1) != args.end())
        {
            cmd.replay_path = *(++found);
            cmd.replay = true;
            args.erase(found, found + 2);
        }

        if (!args.empty())
        {
            std::wostringstream oss;
            std::copy(args.begin(), args.end(), std::ostream_iterator<std::wstring, wchar_t>(oss, L" "));
            auto args_str = oss.str();
            // remove trailing space
            args_str = args_str.substr(0, oss.str().size() - 1);
            const auto message = L"Unknown arguments: " + args_str;
            show_help(message.c_str(), true);
            std::exit(1);
        }
    }
    return cmd;
}
