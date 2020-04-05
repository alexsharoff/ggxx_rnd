#pragma once

#include "command_line.h"
#include "config.h"
#include "game.h"


namespace sound_fix
{

void Initialize(IGame* game, const command_line& cmd);

}
