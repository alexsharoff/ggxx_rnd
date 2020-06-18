#include "binary_patch.h"

#include <fstream>


bool apply_patch_to_file(const wchar_t* path, const X64DbgPatch& patch)
{
    // TODO: validate filename

    std::fstream file(path, std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open())
        return false;
    for (const auto& item : patch.items)
    {
        auto offset = item.offset - 0xc00;
        file.seekg(offset, std::ios::beg);
        uint8_t byte = 0xff;
        if (!file.read(reinterpret_cast<char*>(&byte), 1))
            return false;
        if (byte != item.src)
            return false;
    }
    for (const auto& item : patch.items)
    {
        auto offset = item.offset - 0xc00;
        file.seekp(offset, std::ios::beg);
        if (!file.write(reinterpret_cast<const char*>(&item.dest), 1))
            return false;
    }
    return true;
}

bool apply_patch_in_memory(HMODULE module, const X64DbgPatch& patch)
{
    auto image_base = reinterpret_cast<size_t>(module);
    for (const auto& item : patch.items)
    {
        auto byte_ptr = reinterpret_cast<uint8_t*>(image_base + item.offset);
        if (*byte_ptr != item.src)
            return false;
    }
    for (const auto& item : patch.items)
    {
        auto byte_ptr = reinterpret_cast<uint8_t*>(image_base + item.offset);
        *byte_ptr = item.dest;
    }
    return true;
}
