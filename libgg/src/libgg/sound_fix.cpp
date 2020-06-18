#include "sound_fix.h"

#include "hash.h"

#include <array>
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
struct SoundAccounting
{
    uint32_t frame;
    sound_set_t sounds;
};
// keep only the last 30 frames
std::array<SoundAccounting, 30> g_heard_sounds;

bool play_sound_hook(IGame* game)
{
    if (g_cfg->get_args().unattended)
        return false;

    const auto frame = game->GetState().match.frame.get();
    auto& accounting = g_heard_sounds[frame % g_heard_sounds.size()];
    if (accounting.frame != frame)
    {
        accounting.frame = frame;
        accounting.sounds.clear();
    }
    const auto& sound_id = game->GetCurrentSound();
    auto [_, success] = accounting.sounds.insert(sound_id);
    if (success)
        return true;

    return false;
}

}

bool Initialize(IGame* game, configuration* cfg)
{
    g_cfg = cfg;
    game->RegisterCallback(IGame::Event::BeforePlaySound, play_sound_hook);
    return true;
}

}
