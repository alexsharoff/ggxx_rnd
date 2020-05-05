
#include "unattended.h"

#include "memory_dump.h"


namespace unattended
{

void Initialize(IGame* game, configuration* cfg)
{
    if (cfg->get_args().unattended)
    {
        constexpr uint8_t showwindow_cmd = SW_SHOWMINNOACTIVE;
        memory_dump::dump(showwindow_cmd, game->GetImageBase() + 0x1473B6);

        game->EnableDrawing(false);

        // TODO: disable sound
    }
}

}
