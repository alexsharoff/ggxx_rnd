#pragma once

#include <cstdint>


template<typename T>
struct Fnv1aHashDefaults {};

template<>
struct Fnv1aHashDefaults<uint32_t>
{
    static constexpr uint32_t offset_basis = 2166136261;
    static constexpr uint32_t prime = 16777619;
};

// Fowler–Noll–Vo hash
template<typename HashT = size_t>
class Fnv1aHash
{
public:
    Fnv1aHash() : m_offset_basis(Fnv1aHashDefaults<HashT>::offset_basis) {}

    HashT get() const
    {
        return m_offset_basis;
    }

    Fnv1aHash& add(const uint8_t* data, size_t len)
    {
        for (size_t i = 0; i < len; ++i)
        {
            m_offset_basis ^= static_cast<size_t>(data[i]);
            m_offset_basis *= Fnv1aHashDefaults<HashT>::prime;
        }
        return *this;
    }

    template <class T>
    Fnv1aHash& add(const T& value)
    {
        return add(reinterpret_cast<const uint8_t*>(&value), sizeof(value));
    }
private:
    HashT m_offset_basis;
};
