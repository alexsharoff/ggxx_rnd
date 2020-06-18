#include "libgg.h"

#include <Windows.h>

#include "attach_console.h"
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


extern "C" __declspec(dllexport) bool libgg_init()
{
    static std::shared_ptr<IGame> s_game;
    static std::shared_ptr<configuration> s_cfg;
    if (!s_game)
    {
        // attach cout/cerr to available console when runing from cmd.exe
        attach_console();

        s_cfg = std::make_shared<configuration>();

        const auto image_base = (size_t)::GetModuleHandle(nullptr);
        if (!apply_patches(image_base))
            return false;
        s_game = IGame::Initialize(image_base, s_cfg.get());
        if (!s_game)
            return false;
        if (!unattended::Initialize(s_game.get(), s_cfg.get()))
            return false;
        if (!print_state::Initialize(s_game.get(), s_cfg.get()))
            return false;
        if (!ggpo::Initialize(s_game.get(), s_cfg.get()))
            return false;
        if (!recorder::Initialize(s_game.get(), s_cfg.get()))
            return false;
        if (!training_mode_ex::Initialize(s_game.get(), s_cfg.get()))
            return false;
        if (!skip_intro::Initialize(s_game.get(), s_cfg.get()))
            return false;
        if (!sound_fix::Initialize(s_game.get(), s_cfg.get()))
            return false;
        return true;
    }
    return false;
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
