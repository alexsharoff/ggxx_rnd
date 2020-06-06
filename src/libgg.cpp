#include "libgg.h"

#include <Windows.h>

#include "configuration.h"
#include "game.h"
#include "patches.h"
#include "ggpo.h"
#include "print_state.h"
#include "recorder.h"
#include "training_mode_ex.h"
#include "skip_intro.h"
#include "sound_fix.h"
#include "unattended.h"
#include "util.h"


extern "C" __declspec(dllexport) void libgg_init()
{
    static std::shared_ptr<IGame> s_game;
    static std::shared_ptr<configuration> s_cfg;
    if (!s_game)
    {
        attach_console();

        s_cfg = std::make_shared<configuration>();

        const auto image_base = (size_t)::GetModuleHandle(nullptr);
        apply_patches(image_base);
        s_game = IGame::Initialize(image_base, s_cfg.get());
        unattended::Initialize(s_game.get(), s_cfg.get());
        print_state::Initialize(s_game.get(), s_cfg.get());
        ggpo::Initialize(s_game.get(), s_cfg.get());
        recorder::Initialize(s_game.get(), s_cfg.get());
        training_mode_ex::Initialize(s_game.get(), s_cfg.get());
        skip_intro::Initialize(s_game.get(), s_cfg.get());
        sound_fix::Initialize(s_game.get(), s_cfg.get());
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
