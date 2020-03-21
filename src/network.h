#pragma once

#include "libgg.h"

namespace network
{

extern bool g_is_active;

void activate();

namespace callbacks
{
bool raw_input_data(input_data& input);
bool sleep(uint32_t ms);
bool game_tick_begin();
void game_tick_end();
bool play_sound(const IXACT3WaveBank* bank, int16_t sound_id);
}

}
