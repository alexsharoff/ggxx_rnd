
#include "sound_fix.h"
#include "util.h"

#include <unordered_map>
#include <unordered_set>


// Sound fix for rollbacks (ggpo.cpp) and frame rewind (recorder.cpp)
// TODO: fix voices cutting off at the second frame of playback
// TODO: stop sounds that "never happened" due to rollback
namespace sound_fix
{

namespace
{

configuration* g_cfg;

struct pair_hash
{
    template<class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2>& pair) const
    {
        return Fnv1aHash().add(pair.first).add(pair.second).get();
    }
};
using sound_set_t = std::unordered_set<std::pair<const IXACT3WaveBank*, int16_t>, pair_hash>;
std::unordered_map<size_t, sound_set_t> g_heard_sounds;

bool play_sound_hook(IGame* game)
{
    if (g_cfg->get_args().unattended)
        return false;

    const auto frame = game->GetState().match2.frame.get();
    const auto& sound_id = game->GetCurrentSound();
    auto [_, success] = g_heard_sounds[frame].insert(sound_id);
    if (success)
        return true;

    return false;
}

}

void Initialize(IGame* game, configuration* cfg)
{
    g_cfg = cfg;
    game->RegisterCallback(IGame::Event::BeforePlaySound, play_sound_hook);
}

}
