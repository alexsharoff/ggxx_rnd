#include "patches.h"

#include "memory_dump.h"

#include "patches/config_in_game_dir.h"
#include "patches/display_cfg_shared_mode.h"
#include "patches/multiple_instances.h"
#include "patches/predictable_background_io.h"

#include <binary_patch.h>
#include <memory_protection.h>

#include <cassert>
#include <string>

using memory_dump::dump;

bool apply_patches(size_t image_base)
{
    // GG executable expects current directory to point to installation directory.
    // This may be not be true when the process is started through terminal / IDE.
    std::wstring path;
    {
        wchar_t path_[MAX_PATH];
        const auto size = ::GetModuleFileNameW(NULL, path_, MAX_PATH);
        path_[size] = 0;
        path = path_;
    }
    const auto pos = path.find_last_of('\\');
    const auto dir = path.substr(0, pos);
    if (!::SetCurrentDirectoryW(dir.c_str()))
        return false;

    EnsureMemoryRegionIsWritable scope(image_base + 4096);
    auto module = reinterpret_cast<HMODULE>(image_base);
    if (!apply_patch_in_memory(module, g_patch_config_in_game_dir))
        return false;
    if (!apply_patch_in_memory(module, g_patch_display_cfg_shared_mode))
        return false;
    if (!apply_patch_in_memory(module, g_patch_multiple_instances))
        return false;
    if (!apply_patch_in_memory(module, g_patch_predictable_background_io))
        return false;
    return true;
}
