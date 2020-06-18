#include "patches/load_libgg_dll.h"

#include <PEFile.h>
#include <SteamStub32_Variant3.h>


int wmain(int argc, wchar_t *argv[])
{
    if (argc != 3)
        return 1;

    const auto ggxxacpr_path = argv[1];
    const auto output_path = argv[2];

    PEFile pefile;
    if (!pefile.Initialize(ggxxacpr_path))
        return 2;

    if (!SteamStub32Variant3::ProcessFileEx(&pefile, output_path, false, true, true, false))
        return 3;

    if (!apply_patch_to_file(output_path, g_patch_load_libgg_dll))
        return 4;

    return 0;
}
