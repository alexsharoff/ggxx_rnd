#pragma once

#include <memory_dump.h>
#include <mini_reflection.h>

namespace binary_serializer
{

namespace
{

using mini_reflection::reflect;
using mini_reflection::self;
using mini_reflection::array;
using mini_reflection::member_descr;
using mini_reflection::sizeof_noalign;
using mini_reflection::remove_cvref_t;

//using memory_dump::abs_mem_ptr;
using memory_dump::memory_offset;
using memory_dump::offset_t;

template<class T, class FuncT, class... Args>
constexpr bool for_each_member_logical(T&& o, FuncT& f)
{
    return std::apply(
        [&](const auto&... m)
        {
            return (f(std::forward<T>(o), m) && ... && true);
        },
        reflect<T>::members
    );
}

template<class T>
size_t deserialize(const char* data, size_t size, T& to);

template<class T>
size_t serialize(const T& from, char* data, size_t size);

struct buffer_reader
{
    buffer_reader(const char* buffer, size_t size)
        : m_buffer(buffer), m_size(size), m_offset(0) {}

    template<class T>
    size_t read(T& value)
    {
        static_assert(std::is_standard_layout_v<T>);
        if (m_size - m_offset < sizeof(T))
            return -1;
        value = *reinterpret_cast<const T*>(m_buffer + m_offset);
        m_offset += sizeof(T);
        return sizeof(T);
    }

    size_t offset() const { return m_offset; }

private:
    const char* m_buffer;
    size_t m_size;
    size_t m_offset;
};

struct buffer_writer
{
    buffer_writer(char* buffer, size_t size)
        : m_buffer(buffer), m_size(size), m_offset(0) {}

    template<class T>
    size_t write(const T& value)
    {
        static_assert(std::is_standard_layout_v<T>);
        if (m_size - m_offset < sizeof(T))
            return -1;
        *reinterpret_cast<T*>(m_buffer + m_offset) = value;
        m_offset += sizeof(T);
        return sizeof(T);
    }

    size_t offset() const { return m_offset; }

private:
    char* m_buffer;
    size_t m_size;
    size_t m_offset;
};

template<class SequentialWriterT = buffer_writer>
struct serializer
{
    serializer() = default;
    serializer(const SequentialWriterT& writer) : m_writer(writer) {}
    template<class... Args>
    serializer(Args&&... args) : m_writer(std::forward<Args>(args)...) {}

    template<class T>
    bool operator()(const T& val, const self<T>&)
    {
        return m_writer.write(val) != -1;
    }

    template<class T, size_t Offset>
    bool operator()(const memory_offset<T, Offset>& o,
                    const self<memory_offset<T, Offset>>&)
    {
        return for_each_member_logical(o.get(), *this);
    }

    template<class T>
    bool operator()(const abs_mem_ptr<T>& o,
                    const self<abs_mem_ptr<T>>&)
    {
        if(o)
        {
            if (!(*this)(o.get(), self<offset_t>{}))
                return false;
            return for_each_member_logical(*o, *this);
        }
        else
        {
            return (*this)(static_cast<offset_t>(abs_mem_ptr<T>::invalid_offset), self<offset_t>{});
        }
    }

    template<class T, size_t N>
    bool operator()(const T (&arr)[N], const array<T, N>&)
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (!for_each_member_logical(arr[i], *this))
                return false;
        }
        return true;
    }

    template<class T, class BaseT, class MemberT>
    bool operator()(const T& o, const member_descr<BaseT, MemberT>& m)
    {
        return for_each_member_logical(o.*m.member, *this);
    }

    size_t offset() const
    {
        return m_writer.offset();
    }

private:
    SequentialWriterT m_writer;
};

template<class SequentialReaderT = buffer_reader>
struct deserializer
{
    deserializer() = default;
    deserializer(const SequentialReaderT& reader) : m_reader(reader) {}
    template<class... Args>
    deserializer(Args&&... args) : m_reader(std::forward<Args>(args)...) {}

    template<class T>
    bool operator()(T& val, const self<T>&)
    {
        return m_reader.read(val) != -1;
    }

    // TODO: specialize user types externally
    template<class T, size_t Offset>
    bool operator()(memory_offset<T, Offset>& o,
                    const self<memory_offset<T, Offset>>&)
    {
        return for_each_member_logical(o.get(), *this);
    }

    template<class T>
    bool operator()(abs_mem_ptr<T>& o,
                    const self<abs_mem_ptr<T>>&)
    {
        offset_t offset = abs_mem_ptr<T>::invalid_offset;
        if (!(*this)(offset, self<offset_t>{}))
            return false;
        if (offset != abs_mem_ptr<T>::invalid_offset)
        {
            T value;
            if (!for_each_member_logical(value, *this))
                return false;
            o.reset(value, offset);
        }
        else
        {
            o.reset();
        }
        return true;
    }

    template<class T, size_t N>
    bool operator()(T (&arr)[N], const array<T, N>&)
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (!for_each_member_logical(arr[i], *this))
                return false;
        }
        return true;
    }

    template<class T, class BaseT, class MemberT>
    bool operator()(T& o, const member_descr<BaseT, MemberT>& m)
    {
        return for_each_member_logical(o.*m.member, *this);
    }

    size_t offset() const
    {
        return m_reader.offset();
    }

private:
    SequentialReaderT m_reader;
};

}

template<class T>
struct max_serialized_size
{
    constexpr static size_t value = std::apply(
        [](const auto&... m)
        {
            return (max_serialized_size<remove_cvref_t<decltype(m)>>::value + ... + 0);
        },
        reflect<T>::members
    );
};
template<class T, class MemberT>
struct max_serialized_size<member_descr<T, MemberT>>
{
    constexpr static size_t value = max_serialized_size<remove_cvref_t<MemberT>>::value;
};
template<class T>
struct max_serialized_size<self<T>>
{
    constexpr static size_t value = sizeof(T);
};
template<class T, size_t N>
struct max_serialized_size<array<T, N>>
{
    constexpr static size_t value = max_serialized_size<remove_cvref_t<T>>::value * N;
};
template<class T, offset_t Offset>
struct max_serialized_size<self<memory_offset<T, Offset>>>
{
    constexpr static size_t value = max_serialized_size<T>::value;
};
template<class T>
struct max_serialized_size<self<abs_mem_ptr<T>>>
{
    constexpr static size_t value = max_serialized_size<memory_dump::offset_t>::value + max_serialized_size<T>::value;
};

template<class T, class... Args>
std::pair<bool, size_t> deserialize(T& value, Args&&... args)
{
    deserializer deserializer_(std::forward<Args>(args)...);
    bool success = for_each_member_logical(value, deserializer_);
    return std::make_pair(success, deserializer_.offset());
}

template<class T, class... Args>
std::pair<bool, size_t> serialize(const T& value, Args&&... args)
{
    serializer serializer_(std::forward<Args>(args)...);
    bool success = for_each_member_logical(value, serializer_);
    return std::make_pair(success, serializer_.offset());
}

}
