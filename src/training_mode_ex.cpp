#include "training_mode_ex.h"

#include "util.h"

#include <charconv>


namespace training_mode_ex
{

namespace
{

bool process_objects_hook(IGame* game)
{
    const auto& state = game->GetState();
    const auto is_paused = state.match.pause_state.get();
    const auto display_damage = state.match.training_mode_cfg_display.get() & 2;
    if (game->InMatch() && game->InTrainingMode() && !is_paused && display_damage)
    {
        const auto& character_state = game->GetState().match.character_state;
        auto player_idx = game->GetActivePlayers();
        if (player_idx == 2)
            player_idx = 0;
        else
            player_idx = 1;
        const auto& enemy_char_state = character_state.get()[player_idx];
        const auto stun_accumulator = enemy_char_state.stun_accumulator;
        const auto stun_resistance = enemy_char_state.stun_resistance;
        const auto faint_countdown = enemy_char_state.faint_countdown;

        const int increment_y = 0x18;
        const int key_x = player_idx ? 0x16a : 0x2a;
        const int value_x = player_idx ? 0x21a : 0xda;
        const float key_z = 265;
        const float value_z = 266;
        int y = 0xb8;
        {
            y += increment_y;
            game->WriteCockpitFont("        STUN", key_x, y, key_z, 0xff, 1);
            if (stun_accumulator != 0xffff)
            {
                char str[8];
                auto begin = std::begin(str);
                const auto end = std::end(str);
                begin = std::to_chars(
                    begin, end, stun_accumulator / 100
                ).ptr;
                *begin = '/';
                begin = std::to_chars(
                    begin + 1, end, stun_resistance
                ).ptr;
                *begin = 0;
                game->WriteCockpitFont(str, value_x, y, value_z, 0xff, 1);
            }
            else
            {
                game->WriteCockpitFont("FAINT", value_x, y, value_z, 0xff, 1);
            }

            if (faint_countdown)
            {
                y += increment_y;
                game->WriteCockpitFont("       FAINT", key_x, y, key_z, 0xff, 1);
                {
                    char str[6];
                    format_int(str, faint_countdown);
                    game->WriteCockpitFont(str, value_x, y, value_z, 0xff, 1);
                }
            }
        }
    }
    return true;
}

}

void Initialize(IGame* game, configuration*)
{
    game->RegisterCallback(IGame::Event::AfterProcessObjects, process_objects_hook);
}

}
