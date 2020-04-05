#pragma once

#include "command_line.h"
#include "config.h"
#include "game.h"


namespace recorder
{

void Initialize(IGame* game, recorder_config& cfg, const command_line& cmd);

}
