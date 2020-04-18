#pragma once

#include <deque>
#include <string>
#include <unordered_map>
#include <vector>


struct libgg_args
{
    std::wstring replay_path;
    bool replay_record = false;
    bool replay_check = false;
    bool replay_update = false;
    // when frame <key> ends, take next frame from <value> and restore its state
    std::unordered_map<uint32_t, std::deque<uint32_t>> rollback_map;
    bool nographics = false;
    // TODO: GGPO bug: GGPO crashes if frames == 0 in ggpo_start_synctest
    // Assertion failed at ggpo/lib/ggpo/ring_buffer.h:39
    int synctest_frames = 0;
    std::wstring displaycfg;
    std::wstring savedata;
    std::wstring libggcfg;
    bool usedefaults = false;
    enum class game_mode_t : uint8_t
    {
        default = 0,
        vs2p = 1,
        training = 2,
        network = 3
    } game_mode = game_mode_t::default;
    std::vector<int> printstate;
};
