#include "libgg.h"

#include <Windows.h>

#include "command_line.h"
#include "config.h"
#include "game.h"
#include "patches.h"
#include "ggpo.h"
#include "recorder.h"
#include "rollback_test.h"
#include "training_mode_ex.h"
#include "skip_intro.h"
#include "sound_fix.h"


extern "C" __declspec(dllexport) void libgg_init()
{
    static std::shared_ptr<IGame> s_game;
    if (!s_game)
    {
        const auto cmd = parse_command_line();
        auto cfg = load_config();

        const auto image_base = (size_t)::GetModuleHandle(nullptr);
        apply_patches(image_base);
        s_game = IGame::Initialize(image_base, cmd);
        rollback_test::Initialize(s_game.get(), cmd);
        ggpo::Initialize(s_game.get(), cmd);
        recorder::Initialize(s_game.get(), cfg.recorder, cmd);
        training_mode_ex::Initialize(s_game.get(), cmd);
        skip_intro::Initialize(s_game.get(), cfg.skip_intro, cmd);
        sound_fix::Initialize(s_game.get(), cmd);
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
