#include "print_state.h"

#include "game_state_debug.h"

#include <unordered_set>


namespace print_state
{

namespace
{

std::unordered_set<uint32_t> g_frames;

bool before_advance_frame(IGame* game)
{
    const auto& state = game->GetState();
    if (g_frames.find(state.match.frame.get()) != g_frames.end())
        print_game_state(state);

    return true;
}

}

bool Initialize(IGame* game, configuration* cfg)
{
    for (auto frame : cfg->get_args().printstate)
        g_frames.insert(frame);
    game->RegisterCallback(IGame::Event::BeforeAdvanceFrame, before_advance_frame);
    return true;
}

}
