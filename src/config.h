#pragma once

#include <unordered_map>


struct recorder_config
{
    recorder_config();

    enum class recorder_action : uint32_t
    {
        memory_1 = 1,
        erase = 2,
        forward = 4,
        backward = 8,
        frame_pause = 16
    };

    std::unordered_map<int, recorder_action> key_map;
};

struct skip_intro_config
{
    bool enabled = true;
    uint32_t menu_idx = 7;
};

struct config
{
    recorder_config recorder;
    skip_intro_config skip_intro;
};

config load_config();

void save_config(const config&);
