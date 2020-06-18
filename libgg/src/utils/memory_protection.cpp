#include <memory_protection.h>

#include <cassert>


EnsureMemoryRegionIsWritable::EnsureMemoryRegionIsWritable(size_t address)
{
    auto success = ::VirtualQuery((LPCVOID)address, &m_info, sizeof(m_info));
    (void)success;
    assert(success);
    if (m_info.Protect != PAGE_EXECUTE_READWRITE)
    {
        success = ::VirtualProtect(m_info.BaseAddress, m_info.RegionSize, PAGE_EXECUTE_READWRITE, &m_info.Protect);
        assert(success);
    }
    else
    {
        m_info = {};
    }
}

EnsureMemoryRegionIsWritable::~EnsureMemoryRegionIsWritable()
{
    if (m_info.RegionSize)
    {
        DWORD unused;
        auto success = ::VirtualProtect(m_info.BaseAddress, m_info.RegionSize, m_info.Protect, &unused);
        (void)success;
        assert(success);
    }
}
