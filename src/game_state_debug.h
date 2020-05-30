#pragma once

#include "game_state.h"


uint32_t state_checksum(const game_state& state, bool strict = false);
void print_game_state(const game_state& state);
