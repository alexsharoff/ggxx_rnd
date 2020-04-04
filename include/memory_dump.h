#pragma once

#include "mini_reflection.h"

#include <Windows.h>

#include <cassert>
#include <cstring>
#include <limits>
#include <memory>
#include <optional>

// disallow memory_offset<memory_offset>
// disallow memory_offset arrays

namespace memory_dump
{
using offset_t = std::ptrdiff_t;

// TODO: rename rel_mem_ptr => offset_ptr
// TODO: remove abs_mem_ptr, use memory_offset<T*, size_t...>
// memory_offset<T> with >1 Offsets holds std::optional<T>

namespace
{
using mini_reflection::reflect;
using mini_reflection::member_descr;
using mini_reflection::self;
using mini_reflection::array;
using mini_reflection::sizeof_noalign;
using mini_reflection::remove_cvref_t;
}

template<class T, offset_t RelOffset = 4>
struct rel_mem_ptr
{
    using type = T;
    using type_ptr = type*;

    rel_mem_ptr() = default;

    //std::shared_ptr<type> value;
    std::optional<std::pair<size_t, offset_t>> data;

    void set(size_t base, offset_t value)
    {
        data = std::make_pair(base, value);
    }

    void set(type_ptr value)
    {
        assert(is_valid());
        data->second = reinterpret_cast<int64_t>(value) - (data->first + RelOffset);
    }

    type_ptr get() const
    {
        assert(is_valid());
        return reinterpret_cast<type_ptr>(data->first + RelOffset + data->second);
    }

    operator type_ptr() const
    {
        return get();
    }

    bool is_valid() const
    {
        return !!data;
    }

    operator bool() const
    {
        assert(is_valid());
        return *ptr != nullptr;
    }
};

template<class T>
struct sizeof_noalign<self<rel_mem_ptr<T>>>
{
    constexpr static size_t value = sizeof_noalign<rel_mem_ptr<T>::type_ptr>::value;
};

// offset relative to parent struct, or position in array
template<class T, offset_t Offset>
struct memory_offset
{
    template<class U = T, std::enable_if_t<!std::is_array_v<U>>* = nullptr, class... Args>
    memory_offset(Args&&... args) : data(std::forward<Args>(args)...) {}

    template<class U = T, std::enable_if_t<std::is_array_v<U>>* = nullptr, class V, size_t N>
    memory_offset(const V (&other)[N])
    {
        for (size_t i = 0; i < N; ++i)
            data[i] = other[i];
    }

    template<class U = T, std::enable_if_t<std::is_array_v<U>>* = nullptr>
    memory_offset() {}

    const T& get() const
    {
        return data;
    }
    T& get()
    {
        return data;
    }
    void operator=(T&& value)
    {
        data = std::move(value);
    }
    template<class U = T, std::enable_if_t<!std::is_same_v<U&&, const U&>>* = nullptr>
    void operator=(const T& value)
    {
        data = value;
    }
    template<class U = T, std::enable_if_t<!std::is_same_v<const U, U>>* = nullptr>
    operator const T&() const
    {
        return data;
    }
    operator T&()
    {
        return data;
    }
private:
    T data;
};

template<class T, offset_t Offset>
struct sizeof_noalign<self<memory_offset<T, Offset>>>
{
    constexpr static size_t value = sizeof_noalign<T>::value;
};

template<class T, offset_t Offset>
struct offset_value
{
    template<class... Args>
    offset_value(Args&&... args) : data(std::forward<Args>(args)...) {}

    const T& get() const
    {
        return data;
    }
    T& get()
    {
        return data;
    }
    void operator=(T&& value)
    {
        data = std::move(value);
    }
    template<class U = T, std::enable_if_t<!std::is_same_v<U&&, const U&>>* = nullptr>
    void operator=(const T& value)
    {
        data = value;
    }
    template<class U = T, std::enable_if_t<!std::is_same_v<const U, U>>* = nullptr>
    operator const T&() const
    {
        return data;
    }
    operator T&()
    {
        return data;
    }
private:
    T data;
};


template<class T, size_t Offset, size_t... Args>
struct ptr_chain
{
    std::optional<T> ptr;
};

template<class T, size_t Offset1, size_t Offset2, size_t... Args>
struct ptr_chain<T, Offset1, Offset2, Args...> : public ptr_chain<T, Offset2, Args...>
{
};

namespace
{

template<class T, class FuncT, class... Args>
constexpr void for_each_member_addr(T&& o, FuncT& f, size_t& addr)
{
    size_t addr_m = addr;
    std::apply(
        [&](const auto&... m)
        {
            (f(std::forward<T>(o), m, addr, addr_m), ...);
        },
        reflect<T>::members
    );
    addr = addr_m;
}

struct local_memory_accessor
{
    template<class T>
    static void read(size_t addr, T& value)
    {
        static_assert(std::is_standard_layout_v<T>);
        value = *reinterpret_cast<const T*>(addr);
    }

    template<class T>
    static void write(const T& value, size_t addr)
    {
        const auto ptr = reinterpret_cast<T*>(addr);
        if (!std::memcmp(ptr, &value, sizeof(T)))
            return;
        MEMORY_BASIC_INFORMATION info;
        auto success = ::VirtualQuery(ptr, &info, sizeof(info));
        (void)success;
        assert(success);
        bool cleanup = false;
        if (info.Protect != PAGE_EXECUTE_READWRITE)
        {
            success = ::VirtualProtect(info.BaseAddress, info.RegionSize, PAGE_EXECUTE_READWRITE, &info.Protect);
            assert(success);
            cleanup = true;
        }
        static_assert(std::is_standard_layout_v<T>);
        *ptr = value;
        if (cleanup)
        {
            DWORD unused;
            auto success = ::VirtualProtect(info.BaseAddress, info.RegionSize, info.Protect, &unused);
            (void)success;
            assert(success);
        }
    }

    template<class T, size_t N>
    static void write(const T (&value)[N], size_t addr)
    {
        for (size_t i = 0; i < N; ++i)
            write(value[i], addr + sizeof(value[i]) * i);
    }
};

struct local_memory_accessor_unprotected
{
    template<class T>
    static void read(size_t addr, T& value)
    {
        static_assert(std::is_standard_layout_v<T>);
        value = *reinterpret_cast<const T*>(addr);
    }

    template<class T>
    static void write(const T& value, size_t addr)
    {
        const auto ptr = reinterpret_cast<T*>(addr);
        static_assert(std::is_standard_layout_v<T>);
        *ptr = value;
    }

    template<class T, size_t N>
    static void write(const T (&value)[N], size_t addr)
    {
        for (size_t i = 0; i < N; ++i)
            write(value[i], addr + sizeof(value[i]) * i);
    }
};

template<class MemoryAccessorT = local_memory_accessor>
struct dumper
{
    dumper() = default;
    dumper(const MemoryAccessorT& accessor) : m_accessor(accessor) {}

    template<class T>
    void operator()(const T& val, const self<T>&, size_t /* addr_o */, size_t& addr_m) const
    {
        m_accessor.write(val, addr_m);
        addr_m += sizeof(T);
    }

    // TODO: specialize user types externally
    template<class T, size_t Offset>
    void operator()(const memory_offset<T, Offset>& o,
                    const self<memory_offset<T, Offset>>&,
                    size_t addr_o, size_t& /* addr_m */) const
    {
        size_t addr = addr_o + Offset;
        for_each_member_addr(o.get(), *this, addr);
    }

    template<class T, size_t RelOffset>
    void operator()(const rel_mem_ptr<T, RelOffset>& o,
                    const self<rel_mem_ptr<T, RelOffset>>&,
                    size_t addr_o, size_t& addr_m) const
    {
        assert(o.is_valid());
        (*this)(o.data->second, self<offset_t>{}, addr_o, addr_m);
        //for_each_member_addr(*o, *this, ptr_val);
    }

    template<class T, size_t Offset>
    void operator()(const ptr_chain<T, Offset>& o,
                    const self<ptr_chain<T, Offset>>&,
                    size_t /* addr_o */, size_t& addr_m) const
    {
        if (o.ptr)
        {
            size_t addr_m_o = addr_m + Offset;
            for_each_member_addr(o.ptr.value(), *this, addr_m_o);
        }
    }

    template<class T, size_t Offset, size_t... Args>
    void operator()(const ptr_chain<T, Offset, Args...>& o,
                    const self<ptr_chain<T, Offset, Args...>>&,
                    size_t /* addr_o */, size_t& addr_m) const
    {
        if (o.ptr)
        {
            size_t ptr;
            m_accessor.read(addr_m + Offset, ptr);
            if (ptr)
                (*this)(static_cast<const ptr_chain<T, Args...>&>(o), self<ptr_chain<T, Args...>>{}, ptr, ptr);
        }
        addr_m += sizeof(T*);
    }

    template<class T, size_t N>
    void operator()(const T (&arr)[N], const array<T, N>&, size_t /* addr_o */, size_t& addr_m) const
    {
        for (size_t i = 0; i < N; ++i)
        {
            for_each_member_addr(arr[i], *this, addr_m);
        }
    }

    template<class T, class BaseT, class MemberT>
    void operator()(const T& o, const member_descr<BaseT, MemberT>& m,
                    size_t /* addr_o */, size_t& addr_m) const
    {
        for_each_member_addr(o.*m.member, *this, addr_m);
    }

    template<class T, class BaseT, class T2, offset_t Offset>
    void operator()(const T& o, const member_descr<BaseT, memory_offset<T2, Offset>>& m,
                    size_t addr_o, size_t&) const
    {
        for_each_member_addr(o.*m.member, *this, addr_o);
    }

private:
    MemoryAccessorT m_accessor;
};

template<class MemoryAccessorT = local_memory_accessor>
struct loader
{
    loader() = default;
    loader(const MemoryAccessorT& accessor) : m_accessor(accessor) {}

    template<class T>
    void operator()(T& val, const self<T>&, size_t /* addr_o */, size_t& addr_m) const
    {
        m_accessor.read(addr_m, val);
        addr_m += sizeof(T);
    }

    template<class T, size_t Offset>
    void operator()(memory_offset<T, Offset>& o,
                    const self<memory_offset<T, Offset>>&,
                    size_t addr_o, size_t& /* addr_m */) const
    {
        size_t addr = addr_o + Offset;
        for_each_member_addr(o.get(), *this, addr);
    }

    template<class T, size_t Offset>
    void operator()(ptr_chain<T, Offset>& o,
                    const self<ptr_chain<T, Offset>>&,
                    size_t /* addr_o */, size_t& addr_m) const
    {
        size_t addr_m_o = addr_m + Offset;
        T value;
        for_each_member_addr(value, *this, addr_m_o);
        o.ptr = value;
    }

    template<class T, size_t Offset, size_t... Args>
    void operator()(ptr_chain<T, Offset, Args...>& o,
                    const self<ptr_chain<T, Offset, Args...>>&,
                    size_t addr_o, size_t& addr_m) const
    {
        size_t ptr;
        size_t addr_m_o = addr_m + Offset;
        (*this)(ptr, self<decltype(ptr)>{}, addr_o, addr_m_o);
        if (ptr)
            (*this)(static_cast<ptr_chain<T, Args...>&>(o), self<ptr_chain<T, Args...>>{}, ptr, ptr);
        else
            o.ptr.reset();
        addr_m += sizeof(ptr);
    }

    template<class T, offset_t RelOffset>
    void operator()(rel_mem_ptr<T, RelOffset>& o,
                    const self<rel_mem_ptr<T, RelOffset>>&,
                    size_t addr_o, size_t& addr_m) const
    {
        offset_t ptr;
        size_t base = addr_m;
        (*this)(ptr, self<offset_t>{}, addr_o, addr_m);
        o.set(base, ptr);
        /*T value;
        for_each_member_addr(value, *this, ptr_val);
        o.reset(value, ptr_offset);*/
    }

    template<class T, size_t N>
    void operator()(T (&arr)[N], const array<T, N>&, size_t /* addr_o */, size_t& addr_m) const
    {
        for (size_t i = 0; i < N; ++i)
        {
            for_each_member_addr(arr[i], *this, addr_m);
        }
    }

    template<class T, class BaseT, class MemberT>
    void operator()(T& o, const member_descr<BaseT, MemberT>& m,
                    size_t /* addr_o */, size_t& addr_m) const
    {
        for_each_member_addr(o.*m.member, *this, addr_m);
    }

    template<class T, class BaseT, class T2, offset_t Offset>
    void operator()(T& o, const member_descr<BaseT, memory_offset<T2, Offset>>& m,
                    size_t addr_o, size_t& /* addr_m */) const
    {
        for_each_member_addr(o.*m.member, *this, addr_o);
    }

private:
    MemoryAccessorT m_accessor;
};

}

template<class T>
void load(size_t from, T& to)
{
    static const loader s_loader;
    for_each_member_addr(to, s_loader, from);
}

template<class T1, class T2>
void load(const T1* from, T2& to)
{
    load(reinterpret_cast<size_t>(from), to);
}

template<class T>
void dump(const T& from, size_t to)
{
    static const dumper s_dumper{};
    for_each_member_addr(from, s_dumper, to);
}

template<class T1, class T2>
void dump(const T1& from, T2* to)
{
    dump(from, reinterpret_cast<size_t>(to));
}

template<class T>
void dump_unprotected(const T& from, size_t to)
{
    static const dumper<local_memory_accessor_unprotected> s_dumper{};
    for_each_member_addr(from, s_dumper, to);
}

template<class T1, class T2>
void dump_unprotected(const T1& from, T2* to)
{
    dump_unprotected(from, reinterpret_cast<size_t>(to));
}

}
