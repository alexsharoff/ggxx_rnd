#pragma once

#include <string>


struct command_line
{
    std::wstring replay_path;
    bool replay = false;
    bool record = false;
};

command_line parse_command_line();
