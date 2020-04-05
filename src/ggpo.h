#pragma once

#include "command_line.h"
#include "config.h"
#include "game.h"


namespace ggpo
{

void Initialize(IGame* game, const command_line& cmd);

}
