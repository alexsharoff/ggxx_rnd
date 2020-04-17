#pragma once

#include "command_line.h"
#include "config.h"
#include "game.h"


namespace skip_intro
{

void Initialize(IGame* game, const skip_intro_config& cfg, const command_line& cmd);

}
