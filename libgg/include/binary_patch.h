#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <Windows.h>


struct X64DbgPatch
{
    std::string filename;

    struct Item
    {
        size_t offset;
        uint8_t src;
        uint8_t dest;
    };
    std::vector<Item> items;
};

bool apply_patch_to_file(const wchar_t* path, const X64DbgPatch& patch);

bool apply_patch_in_memory(HMODULE module, const X64DbgPatch& patch);
