#pragma once

#include <unordered_map>

#include <Windows.h>


struct recorder_settings
{
    enum class recorder_action : uint16_t
    {
        memory_1 = 1,
        memory_2 = 2,
        memory_3 = 4,
        memory_4 = 8,
        erase = 16,
        forward = 32,
        backward = 64,
        frame_pause = 128
    };

    std::pair<uint8_t, recorder_action> keyboard_config[16] = {
        { (uint8_t)VK_SPACE, recorder_action::frame_pause },
        { (uint8_t)VK_PRIOR, recorder_action::backward },
        { (uint8_t)VK_NEXT, recorder_action::forward },
        { (uint8_t)VK_F1, recorder_action::memory_1 },
        { (uint8_t)VK_F2, recorder_action::memory_2 },
        { (uint8_t)VK_F3, recorder_action::memory_3 },
        { (uint8_t)VK_F4, recorder_action::memory_4 },
        { (uint8_t)VK_SHIFT, recorder_action::erase }
    };
};

struct skip_intro_settings
{
    // TODO: defaults should not change game behavior
    uint8_t enabled = true;
    uint8_t menu_idx = 7;
    uint8_t enter_menu = false;
};
