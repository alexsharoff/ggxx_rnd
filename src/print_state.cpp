
#include "print_state.h"

#include <unordered_set>


namespace print_state
{

namespace
{

std::unordered_set<uint32_t> g_frames;

bool before_game_tick_hook(IGame* game)
{
    const auto& state = game->GetState();
    if (g_frames.find(state.match2.frame.get()) != g_frames.end())
        print_game_state(state);

    return true;
}

}

void Initialize(IGame* game, configuration* cfg)
{
    for (auto frame : cfg->get_args().printstate)
        g_frames.insert(frame);
    game->RegisterCallback(IGame::Event::BeforeGameTick, before_game_tick_hook);
}

}
