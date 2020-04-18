#include "rollback_test.h"

#include "game_state.h"

#include <unordered_map>
#include <utility>


namespace rollback_test
{

namespace
{

libgg_args g_cmd;
std::unordered_map<uint32_t, std::optional<game_state>> g_state_map;

bool input_hook(IGame* game)
{
    const auto frame = game->GetState().match2.clock.get();
    {
        auto found = g_state_map.find(frame);
        if (found != g_state_map.end())
        {
            found->second = game->GetState();
        }
    }
    {
        auto found = g_cmd.rollback_map.find(frame);
        if (found != g_cmd.rollback_map.end())
        {
            const auto rollback_frame = found->second.front();
            found->second.pop_front();
            game->SetState(*g_state_map[rollback_frame]);
            if (found->second.empty())
                g_cmd.rollback_map.erase(found);
        }
    }
    return true;
}

}

void Initialize(IGame* game, configuration* cfg)
{
    g_cmd = cfg->get_args();
    for (const auto& [from, to] : g_cmd.rollback_map)
    {
        for (auto frame : to)
            g_state_map[frame] = std::nullopt;
    }
    game->RegisterCallback(IGame::Event::AfterGetInput, input_hook);
}

}
