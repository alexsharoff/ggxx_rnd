#pragma once

#include <unordered_map>

#include <Windows.h>


struct keyboard_mapping
{
    enum class action : uint16_t
    {
        frame_advance = 1,
        frame_next = 2,
        frame_prev = 4,
        frame_clear_input = 8,
        save_replay = 16
    };

    std::pair<uint8_t, action> mapping[16] = {
        { (uint8_t)VK_SPACE, action::frame_advance },
        { (uint8_t)VK_NEXT, action::frame_next },
        { (uint8_t)VK_PRIOR, action::frame_prev },
        { (uint8_t)VK_SHIFT, action::frame_clear_input },
        { (uint8_t)VK_F1, action::save_replay }
    };
};

struct skip_intro_settings
{
    uint8_t enabled = false;
    uint8_t menu_idx = 0;
};

struct manual_frame_advance_settings
{
    // TODO: disable by default
    uint8_t enabled = true;
};
