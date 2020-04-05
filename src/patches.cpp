#include "patches.h"

#include "memory_dump.h"

#include <algorithm>
#include <string>


using memory_dump::dump;

void apply_patches(size_t /* image_base */)
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
    ::SetCurrentDirectoryW(dir.c_str());

    // TODO: apply *.1337 patches from /Patches subdirectory
}
