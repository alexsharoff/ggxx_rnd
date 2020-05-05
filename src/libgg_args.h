#pragma once

#include <deque>
#include <optional>
#include <string>
#include <vector>


struct libgg_args
{
    struct replay_t
    {
        std::wstring path;
        enum class mode_t : uint8_t
        {
            play = 1,
            record = 2,
            append = 3
        } mode;
    };
    std::optional<replay_t> replay;
    bool unattended = false;
    int synctest_frames = 0;
    enum class game_mode_t : uint8_t
    {
        vs2p = 1,
        training = 2,
        network = 3
    };
    std::optional<game_mode_t> game_mode;
    std::vector<int> printstate;
    struct network_t
    {
        uint8_t side = 1;
        uint16_t localport = 7500;
        uint16_t remoteport = 7501;
        std::string remoteip = "127.0.0.1";
    } network;
};
