#include "config.h"

#include <fstream>

#include <Windows.h>


recorder_config::recorder_config()
{
    key_map = {
        { VK_SPACE, recorder_action::frame_pause },
        { VK_PRIOR, recorder_action::backward },
        { VK_NEXT, recorder_action::forward },
        // TODO: support multiple recordings
        { VK_F1, recorder_action::memory_1 },
        { VK_SHIFT, recorder_action::erase }
    };
}

config load_config()
{
    config cfg;

    // TODO: implement
    //std::ifstream f("libgg.cfg", std::ios::binary);
    //if (f.is_open())
    //    f.read(reinterpret_cast<char*>(&cfg), sizeof(cfg));

    return cfg;
}

void save_config(const config& cfg)
{
    // TODO: implement
    //std::ofstream f("libgg.cfg", std::ios::binary);
    //if (f.is_open())
    //    f.write(reinterpret_cast<const char*>(&cfg), sizeof(cfg));
}
