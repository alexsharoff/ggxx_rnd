
#include "unattended.h"

#include "memory_dump.h"


namespace unattended
{

namespace
{

bool before_draw_frame(IGame*)
{
    // don't draw current frame
    return false;
}

}

void Initialize(IGame* game, configuration* cfg)
{
    if (cfg->get_args().unattended)
    {
        constexpr uint8_t jmp[] = { 0xe9, 0x38, 0x01, 0x00, 0x00 };
        memory_dump::dump(jmp, game->GetImageBase() + 0x1473D7);

        constexpr uint8_t showwindow_cmd = SW_SHOWMINNOACTIVE;
        memory_dump::dump(showwindow_cmd, game->GetImageBase() + 0x1473B6);

        game->RegisterCallback(IGame::Event::BeforeDrawFrame, before_draw_frame);
    }
}

}
