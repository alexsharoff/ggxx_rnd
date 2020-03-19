#pragma once

#include "libgg.h"

namespace network
{

void start_session();
void close_session();

namespace callbacks
{
bool raw_input_data(input_data& input);
bool sleep(uint32_t ms);
bool game_tick_end();
bool play_sound(const IXACT3WaveBank* bank, int16_t sound_id);
}

}
