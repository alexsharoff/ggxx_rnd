#pragma once

#include "libgg.h"

namespace network
{

void start();
void stop();
void process_input(const input_data& input);
// TODO:
// gg calls sleep is called before Present(),
// but it must be called before process_input, process_objects etc
void game_tick_end(char game_state, int idle_timeout_ms);

}
