#include "skip_intro.h"

#include "memory_dump.h"


namespace skip_intro
{

namespace
{

configuration* g_cfg;
skip_intro_settings g_settings;
std::optional<uint32_t> skip_frame;
bool g_user_control = true;
bool g_enter_menu = false;

void enter_menu_automatically(size_t image_base, bool enable = true)
{
    const auto instr_offset = image_base + 0x226233;
    if (enable)
    {
        uint8_t instr[] = {0x90, 0x90}; // nop nop
        memory_dump::dump(instr, instr_offset);
        g_user_control = false;
    }
    else
    {
        uint8_t instr[] = {0x74, 0x49}; // jne +49
        memory_dump::dump(instr, instr_offset);
        g_user_control = true;
    }
}

bool get_input_hook(IGame* game)
{
    if (!g_user_control)
    {
        game->SetCachedInput(IGame::input_t{});
        // remaining callbacks should be skipped
        return false;
    }

    return true;
}

bool game_tick_hook(IGame* game)
{
    if (!g_user_control && !game->FindFiberByName("OPTION"))
    {
        enter_menu_automatically(game->GetImageBase(), false);
    }

    if (g_settings.enabled)
    {
        const auto frame = game->GetState().match.frame.get();
        if (!skip_frame.has_value() || skip_frame == frame)
        {
            const auto& next_fiber = game->GetState().match.next_fiber_id.get();
            if (next_fiber == fiber_id::title)
            {
                auto state = game->GetState();
                state.match.main_menu_idx = g_settings.menu_idx;
                if (g_enter_menu)
                    enter_menu_automatically(game->GetImageBase());
                state.match.next_fiber_id = fiber_id::main_menu;
                game->SetState(state);
                skip_frame = frame;
            }
        }
        else if (g_settings.enabled && game->FindFiberByName("OPTION"))
        {
            // TODO: set menu_idx, save skip_intro_settings o file
            //cfg.menu_idx = (uint8_t)game->GetState().match.main_menu_idx.get();
        }
    }

    return true;
}

}

bool Initialize(IGame* game, configuration* cfg)
{
    g_cfg = cfg;
    g_settings = g_cfg->get_skip_intro_settings();
    const auto& args = g_cfg->get_args();
    if (args.game_mode.has_value())
    {
        g_enter_menu = true;
        g_settings.enabled = true;
        if (args.game_mode == libgg_args::game_mode_t::network)
            g_settings.menu_idx = 3;
        else if (args.game_mode == libgg_args::game_mode_t::training)
            g_settings.menu_idx = 7;
        else if (args.game_mode == libgg_args::game_mode_t::vs2p)
            g_settings.menu_idx = 2;
    }
    game->RegisterCallback(IGame::Event::AfterReadInput, get_input_hook);
    game->RegisterCallback(IGame::Event::BeforeAdvanceFrame, game_tick_hook);
    return true;
}

}
