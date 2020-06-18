#pragma once

#include <Windows.h>


class EnsureMemoryRegionIsWritable
{
public:
    EnsureMemoryRegionIsWritable(size_t address);
    ~EnsureMemoryRegionIsWritable();
private:
    MEMORY_BASIC_INFORMATION m_info;
};
