
#include "sound_fix.h"

#include <unordered_map>
#include <unordered_set>
#include <utility>


// Sound fix for rollbacks (ggpo.cpp) and frame rewind (recorder.cpp)
namespace sound_fix
{

namespace
{

struct pair_hash
{
    template<class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2>& pair) const
    {
        return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
    }
};
using sound_set_t = std::unordered_set<std::pair<const IXACT3WaveBank*, int16_t>, pair_hash>;
std::unordered_map<size_t, sound_set_t> g_heard_sounds;

bool play_sound_hook(IGame* game)
{
    const auto frame = game->GetState().match2.clock.get();
    const auto& sound_id = game->GetCurrentSound();
    auto [_, success] = g_heard_sounds[frame].insert(sound_id);
    if (success)
        return true;

    return false;
}

}

void Initialize(IGame* game, configuration*)
{
    game->RegisterCallback(IGame::Event::BeforePlaySound, play_sound_hook);
}

}
