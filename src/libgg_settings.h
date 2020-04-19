#pragma once

#include <unordered_map>

#include <Windows.h>


struct keyboard_mapping
{
    enum class action : uint16_t
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

    std::pair<uint8_t, action> mapping[16] = {
        { (uint8_t)VK_SPACE, action::frame_pause },
        { (uint8_t)VK_PRIOR, action::backward },
        { (uint8_t)VK_NEXT, action::forward },
        { (uint8_t)VK_F1, action::memory_1 },
        { (uint8_t)VK_F2, action::memory_2 },
        { (uint8_t)VK_F3, action::memory_3 },
        { (uint8_t)VK_F4, action::memory_4 },
        { (uint8_t)VK_SHIFT, action::erase }
    };
};

struct skip_intro_settings
{
    // TODO: defaults should not change game behavior
    uint8_t enabled = true;
    uint8_t menu_idx = 7;
    uint8_t enter_menu = false;
};
