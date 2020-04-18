#include "skip_intro.h"

#include "memory_dump.h"


namespace skip_intro
{

namespace
{

configuration* g_cfg;
bool g_skipped = false;
bool g_restore_instruction = false;

void enter_menu_automatically(size_t image_base, bool enable = true)
{
    const auto instr_offset = image_base + 0x226233;
    if (enable)
    {
        uint8_t instr[] = {0x90, 0x90}; // nop nop
        memory_dump::dump(instr, instr_offset);
        g_restore_instruction = true;
    }
    else
    {
        uint8_t instr[] = {0x74, 0x49}; // jne +49
        memory_dump::dump(instr, instr_offset);
    }
}

bool game_tick_hook(IGame* game)
{
    if (g_restore_instruction && !game->FindFiberByName("OPTION"))
    {
        enter_menu_automatically(game->GetImageBase(), false);
        g_restore_instruction = false;
    }

    const auto& cfg = g_cfg->get_skip_intro_settings();
    if (cfg.enabled)
    {
        if (!g_skipped)
        {
            const auto& next_fiber = game->GetState().match2.next_fiber_id.get();
            if (next_fiber == fiber_id::title)
            {
                auto state = game->GetState();
                state.match2.main_menu_idx = cfg.menu_idx;
                if (cfg.enter_menu)
                    enter_menu_automatically(game->GetImageBase());
                state.match2.next_fiber_id = fiber_id::main_menu;
                game->SetState(state);
                g_skipped = true;
            }
        }
        else if (cfg.enabled && game->FindFiberByName("OPTION"))
        {
            // TODO: set menu_idx, save skip_intro_settings o file
            //cfg.menu_idx = (uint8_t)game->GetState().match2.main_menu_idx.get();
        }
    }

    return true;
}

}

void Initialize(IGame* game, configuration* cfg)
{
    g_cfg = cfg;
    game->RegisterCallback(IGame::Event::BeforeGameTick, game_tick_hook);
}

}
