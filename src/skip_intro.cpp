#include "skip_intro.h"

#include "memory_dump.h"


namespace skip_intro
{

namespace
{

skip_intro_config g_cfg;
command_line g_cmd;
bool g_skipped = false;
bool g_restore_instruction = false;

void enter_charselect_screen_automatically(size_t image_base, bool enable = true)
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
    if (g_restore_instruction && game->FindFiberByName("CHRSLCT"))
    {
        enter_charselect_screen_automatically(game->GetImageBase(), false);
        g_restore_instruction = false;
    }

    if (g_cfg.enabled || g_cmd.game_mode != command_line::game_mode_t::default)
    {
        if (!g_skipped)
        {
            const auto& next_fiber = game->GetState().match2.next_fiber_id.get();
            if (next_fiber == fiber_id::title)
            {
                auto state = game->GetState();
                if (g_cmd.game_mode == command_line::game_mode_t::default)
                {
                    state.match2.main_menu_idx = g_cfg.menu_idx;
                }
                else if (g_cmd.game_mode == command_line::game_mode_t::network)
                {
                    state.match2.main_menu_idx = 3;
                    enter_charselect_screen_automatically(game->GetImageBase());
                }
                else if (g_cmd.game_mode == command_line::game_mode_t::training)
                {
                    state.match2.main_menu_idx = 7;
                    enter_charselect_screen_automatically(game->GetImageBase());
                }
                else if (g_cmd.game_mode == command_line::game_mode_t::vs2p)
                {
                    state.match2.main_menu_idx = 2;
                    enter_charselect_screen_automatically(game->GetImageBase());
                }
                state.match2.next_fiber_id = fiber_id::main_menu;
                game->SetState(state);
                g_skipped = true;
            }
        }
        else if (g_cfg.enabled && game->FindFiberByName("OPTION"))
        {
            g_cfg.menu_idx = game->GetState().match2.main_menu_idx.get();
        }
    }

    return true;
}

}

void Initialize(IGame* game, const skip_intro_config& cfg, const command_line& cmd)
{
    g_cfg = cfg;
    g_cmd = cmd;
    game->RegisterCallback(IGame::Event::BeforeGameTick, game_tick_hook);
}

}
