#include "memory_dump.h"

#include <array>
#include <cstdint>

#include "test.h"

using mini_reflection::reflect;
using mini_reflection::member_tuple;
using mini_reflection::sizeof_noalign;
using mini_reflection::sizeof_noalign;

using memory_dump::load;
using memory_dump::dump;
using memory_dump::memory_offset;
using memory_dump::ptr_chain;

static_assert(sizeof_noalign<memory_offset<uint16_t, 4>>::value == sizeof_noalign<uint16_t>::value);
static_assert(sizeof_noalign<memory_offset<uint16_t, 4>&>::value == sizeof_noalign<uint16_t>::value);
static_assert(sizeof_noalign<memory_offset<uint16_t, 4>&&>::value == sizeof_noalign<uint16_t>::value);
static_assert(sizeof_noalign<const memory_offset<uint16_t, 4>>::value == sizeof_noalign<uint16_t>::value);
static_assert(sizeof_noalign<const memory_offset<uint16_t, 4>&>::value == sizeof_noalign<uint16_t>::value);
static_assert(sizeof_noalign<const memory_offset<uint16_t, 4>&&>::value == sizeof_noalign<uint16_t>::value);
static_assert(sizeof_noalign<volatile memory_offset<uint16_t, 4>>::value == sizeof_noalign<uint16_t>::value);
static_assert(sizeof_noalign<volatile memory_offset<uint16_t, 4>&>::value == sizeof_noalign<uint16_t>::value);
static_assert(sizeof_noalign<volatile memory_offset<uint16_t, 4>&&>::value == sizeof_noalign<uint16_t>::value);
static_assert(sizeof_noalign<const volatile memory_offset<uint16_t, 4>>::value == sizeof_noalign<uint16_t>::value);
static_assert(sizeof_noalign<const volatile memory_offset<uint16_t, 4>&>::value == sizeof_noalign<uint16_t>::value);
static_assert(sizeof_noalign<const volatile memory_offset<uint16_t, 4>&&>::value == sizeof_noalign<uint16_t>::value);
static_assert(sizeof_noalign<memory_offset<uint16_t&, 4>>::value == sizeof_noalign<uint16_t>::value);
static_assert(sizeof_noalign<memory_offset<uint16_t&&, 4>>::value == sizeof_noalign<uint16_t>::value);
static_assert(sizeof_noalign<memory_offset<const uint16_t, 4>>::value == sizeof_noalign<uint16_t>::value);
static_assert(sizeof_noalign<memory_offset<const uint16_t&, 4>>::value == sizeof_noalign<uint16_t>::value);
static_assert(sizeof_noalign<memory_offset<const uint16_t&&, 4>>::value == sizeof_noalign<uint16_t>::value);
static_assert(sizeof_noalign<memory_offset<volatile uint16_t, 4>>::value == sizeof_noalign<uint16_t>::value);
static_assert(sizeof_noalign<memory_offset<volatile uint16_t&, 4>>::value == sizeof_noalign<uint16_t>::value);
static_assert(sizeof_noalign<memory_offset<volatile uint16_t&&, 4>>::value == sizeof_noalign<uint16_t>::value);
static_assert(sizeof_noalign<memory_offset<const volatile uint16_t, 4>>::value == sizeof_noalign<uint16_t>::value);
static_assert(sizeof_noalign<memory_offset<const volatile uint16_t&, 4>>::value == sizeof_noalign<uint16_t>::value);
static_assert(sizeof_noalign<memory_offset<const volatile uint16_t&&, 4>>::value == sizeof_noalign<uint16_t>::value);

struct A
{
    uint16_t n;
    char c_arr[2];
};

#pragma pack(push, 1)
struct A_pack
{
    uint16_t n;
    char c_arr[2];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct A_pack_offset
{
    char offset[4];
    A_pack a_pack;
};
#pragma pack(pop)

template<>
struct reflect<A>
{
    constexpr static auto members = member_tuple(
        &A::n,
        &A::c_arr);
};

static_assert(sizeof_noalign<A>::value == sizeof(A_pack));

static_assert(sizeof_noalign<memory_offset<A, 4>>::value == sizeof_noalign<A>::value);
static_assert(sizeof_noalign<memory_offset<A&, 4>>::value == sizeof_noalign<A>::value);
static_assert(sizeof_noalign<memory_offset<A&&, 4>>::value == sizeof_noalign<A>::value);
static_assert(sizeof_noalign<memory_offset<const A, 4>>::value == sizeof_noalign<A>::value);
static_assert(sizeof_noalign<memory_offset<const A&, 4>>::value == sizeof_noalign<A>::value);
static_assert(sizeof_noalign<memory_offset<const A&&, 4>>::value == sizeof_noalign<A>::value);
static_assert(sizeof_noalign<memory_offset<volatile A, 4>>::value == sizeof_noalign<A>::value);
static_assert(sizeof_noalign<memory_offset<volatile A&, 4>>::value == sizeof_noalign<A>::value);
static_assert(sizeof_noalign<memory_offset<volatile A&&, 4>>::value == sizeof_noalign<A>::value);
static_assert(sizeof_noalign<memory_offset<volatile const A, 4>>::value == sizeof_noalign<A>::value);
static_assert(sizeof_noalign<memory_offset<volatile const A&, 4>>::value == sizeof_noalign<A>::value);
static_assert(sizeof_noalign<memory_offset<volatile const A&&, 4>>::value == sizeof_noalign<A>::value);

struct B_data
{
    A* a_ptr1;
    A* a_ptr2;
    A* a_ptr3;
};

struct B_savestate
{
    A* a_ptr1;
    A* a_ptr2;
    A* a_ptr3;

    memory_offset<ptr_chain<A, 0, 0>, sizeof(A*) * 2> a_offset;
    ptr_chain<std::array<A, 3>, sizeof(A*), 0> a_arr;
    ptr_chain<A, 0, 0> a;
};

template<>
struct reflect<B_savestate>
{
    constexpr static auto members = member_tuple(
        &B_savestate::a_ptr1,
        &B_savestate::a_ptr2,
        &B_savestate::a_ptr3,
        &B_savestate::a,
        &B_savestate::a_arr,
        &B_savestate::a_offset);
};

int main()
{
    A a_arr[5] = {
        { 1, { 'a', 'b' } },
        { 2, { 'c', 'd' } },
        { 3, { 'e', 'f' } },
        { 4, { 'g', 'h' } },
        { 5, { 'i', 'j' } }
    };

    B_data b = { a_arr, a_arr + 1, a_arr + 4 };

    B_savestate savestate;
    load(&b, savestate);

    for (auto& a : a_arr)
    {
        a.n = 0;
        a.c_arr[0] = 0;
        a.c_arr[1] = 0;
    }
    b.a_ptr1 = nullptr;
    b.a_ptr2 = nullptr;
    b.a_ptr2 = nullptr;

    dump(savestate, &b);

    TEST_EQ(b.a_ptr1, a_arr);
    TEST_EQ(b.a_ptr2, a_arr + 1);
    TEST_EQ(b.a_ptr3, a_arr + 4);
    TEST_EQ(a_arr[0].n, 1);
    TEST_EQ(a_arr[0].c_arr[0], 'a');
    TEST_EQ(a_arr[3].n, 4);
    TEST_EQ(a_arr[3].c_arr[1], 'h');
    TEST_EQ(a_arr[4].n, 5);
    TEST_EQ(a_arr[4].c_arr[0], 'i');

    return 0;
}
