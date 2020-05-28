#pragma once

#include <iostream>
#include <system_error>
#include <type_traits>
#include <utility>


#ifdef LIBGG_LOG_ENABLE
#define LIBGG_LOG() std::cout << __FUNCTION__ << ": "
#else
namespace
{
struct dev_null_t
{
    template<typename T>
    dev_null_t& operator<<(const T&) { return *this; }
    template<typename T, size_t N>
    dev_null_t& operator<<(const T(&)[N]) { return *this; }
    // for std::endl et al.
    dev_null_t& operator<<(std::ostream&(*)(std::ostream&)) { return *this; }
};
}
#define LIBGG_LOG() dev_null_t()
#endif

template<size_t N, std::enable_if_t<(N > 1)>* = nullptr>
std::pair<char*, std::errc> format_int(char (&buffer)[N], int value, size_t pad = 3, char pad_c = ' ')
{
    auto [p, ec] = std::to_chars(buffer, buffer + N - 1, value);
    if (ec != std::errc())
        return {nullptr, ec};
    char* end = p;
    const size_t len = end - buffer;
    if (len < pad)
    {
        const size_t diff = pad - len;
        end += diff;
        size_t i = pad;
        do
        {
            --i;
            if (i < diff)
                buffer[i] = pad_c;
            else
                buffer[i] = buffer[i - diff];
        }
        while (i != 0);
    }
    *end = 0;

    return {end, std::errc()};
}

inline uint16_t reverse_bytes(uint16_t value)
{
    return value / 256 + (value & 0xff) * 256;
}

void show_message_box(const wchar_t* message, bool error = false);

void attach_console(bool create_console_if_not_found = false);

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
