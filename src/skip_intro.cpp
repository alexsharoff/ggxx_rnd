#include "skip_intro.h"


namespace skip_intro
{

namespace
{

bool g_skip_intro = true;
bool game_tick_hook(IGame* game)
{
    if (g_skip_intro)
    {
        const auto& next_fiber = game->GetState().match2.next_fiber_id.get();
        if (next_fiber == fiber_id::title)
        {
            auto state = game->GetState();
            state.match2.main_menu_idx = main_menu_idx::training;
            state.match2.next_fiber_id = fiber_id::main_menu;
            game->SetState(state);
            g_skip_intro = false;
        }
    }

    return true;
}

}

void Initialize(IGame* game, const command_line&)
{
    game->RegisterCallback(IGame::Event::BeforeGameTick, game_tick_hook);
}

}
