#include "libgg.h"

#include <Windows.h>

#include "game.h"
#include "patches.h"
#include "ggpo.h"
#include "recorder.h"
#include "training_mode_ex.h"
#include "skip_intro.h"
#include "sound_fix.h"


extern "C" __declspec(dllexport) void libgg_init()
{
    static std::shared_ptr<IGame> s_game;
    if (!s_game)
    {
        const auto image_base = (size_t)::GetModuleHandle(nullptr);
        apply_patches(image_base);
        s_game = IGame::Initialize(image_base);
        ggpo::Initialize(s_game.get());
        recorder::Initialize(s_game.get());
        training_mode_ex::Initialize(s_game.get());
        skip_intro::Initialize(s_game.get());
        sound_fix::Initialize(s_game.get());
    }
}

BOOL WINAPI DllMain(HINSTANCE, DWORD dwReason, LPVOID)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    };
    return TRUE;
}
