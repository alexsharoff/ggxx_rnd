#include "memory_dump.h"

#include <cstdint>

#include "test.h"

using mini_reflection::reflect;
using mini_reflection::member_tuple;
using mini_reflection::sizeof_noalign;

using memory_dump::load;
using memory_dump::dump;
using memory_dump::memory_offset;
using memory_dump::abs_mem_ptr;

static_assert(sizeof_noalign<abs_mem_ptr<uint16_t>>::value == sizeof_noalign<size_t>::value);
static_assert(sizeof_noalign<abs_mem_ptr<uint16_t>&>::value == sizeof_noalign<size_t>::value);
static_assert(sizeof_noalign<abs_mem_ptr<uint16_t>&&>::value == sizeof_noalign<size_t>::value);
static_assert(sizeof_noalign<const abs_mem_ptr<uint16_t>>::value == sizeof_noalign<size_t>::value);
static_assert(sizeof_noalign<const abs_mem_ptr<uint16_t>&>::value == sizeof_noalign<size_t>::value);
static_assert(sizeof_noalign<const abs_mem_ptr<uint16_t>&&>::value == sizeof_noalign<size_t>::value);
static_assert(sizeof_noalign<volatile abs_mem_ptr<uint16_t>>::value == sizeof_noalign<size_t>::value);
static_assert(sizeof_noalign<volatile abs_mem_ptr<uint16_t>&>::value == sizeof_noalign<size_t>::value);
static_assert(sizeof_noalign<volatile abs_mem_ptr<uint16_t>&&>::value == sizeof_noalign<size_t>::value);
static_assert(sizeof_noalign<const volatile abs_mem_ptr<uint16_t>>::value == sizeof_noalign<size_t>::value);
static_assert(sizeof_noalign<const volatile abs_mem_ptr<uint16_t>&>::value == sizeof_noalign<size_t>::value);
static_assert(sizeof_noalign<const volatile abs_mem_ptr<uint16_t>&&>::value == sizeof_noalign<size_t>::value);
static_assert(sizeof_noalign<abs_mem_ptr<volatile uint16_t>>::value == sizeof_noalign<size_t>::value);

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

static_assert(sizeof_noalign<abs_mem_ptr<A>>::value == sizeof_noalign<size_t>::value);
static_assert(sizeof_noalign<abs_mem_ptr<volatile A>>::value == sizeof_noalign<size_t>::value);

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

struct B
{
    A a;
    double d;
    A a_arr[2];
};

#pragma pack(push, 1)
struct B_pack
{
    double d;
    A_pack a;
    A_pack a_arr[2];
};
#pragma pack(pop)

template<>
struct reflect<B>
{
    constexpr static auto members = member_tuple(
        &B::d,
        &B::a,
        &B::a_arr);
};

static_assert(sizeof_noalign<B>::value == sizeof(B_pack));

struct C
{
    abs_mem_ptr<A> a_ptr;
    memory_offset<A, sizeof(A*)> a_offset;
};

template<>
struct reflect<C>
{
    constexpr static auto members = member_tuple(
        &C::a_ptr,
        &C::a_offset);
};

#pragma pack(push, 1)
struct C_pack
{
    A_pack* a_ptr;
    A_pack a;
};
#pragma pack(pop)

struct D
{
    abs_mem_ptr<memory_offset<uint32_t, 4>> offset_ptr;
    memory_offset<abs_mem_ptr<uint32_t>, 0> ptr_offset;
};

template<>
struct reflect<D>
{
    constexpr static auto members = member_tuple(
        &D::ptr_offset,
        &D::offset_ptr);
};

#pragma pack(push, 1)
struct D_pack
{
    uint32_t* n_ptr;
    uint32_t n_arr[3];
};
#pragma pack(pop)

struct E
{
    abs_mem_ptr<A[2]> arr_ptr;
    memory_offset<A[2], sizeof(A*)> arr_offset;
};

template<>
struct reflect<E>
{
    constexpr static auto members = member_tuple(
        &E::arr_ptr,
        &E::arr_offset);
};

#pragma pack(push, 1)
struct E_pack
{
    A_pack* a_ptr;
    A_pack a_arr[4];
};
#pragma pack(pop)

struct F
{
    abs_mem_ptr<F> ptr_arr[2];
    memory_offset<A[2], sizeof(F*) * 2> offset_arr;
};

template<>
struct reflect<F>
{
    constexpr static auto members = member_tuple(
        &F::ptr_arr,
        &F::offset_arr
    );
};

#pragma pack(push, 1)
struct F_pack
{
    F_pack* f_ptr[2];
    A_pack a_arr[2];
};
#pragma pack(pop)

int main()
{
    {
        A_pack a_pack{65535, {'a', 'b'}};
        A a{};
        load(&a_pack, a);
        TEST_EQ(65535, a.n);
        TEST_EQ('b', a.c_arr[1]);

        a.n = 111;
        a.c_arr[1] = 'z';
        dump(a, &a_pack);
        TEST_EQ(111, a_pack.n);
        TEST_EQ('z', a_pack.c_arr[1]);
    }
    {
        B_pack b_pack
        {
            2.1,
            {1, {'a', 'b'}},
            {
                {3, {'c', 'd'}},
                {4, {'e', 'f'}}
            }
        };
        B b{};
        load(&b_pack, b);
        TEST_EQ(1, b.a.n);
        TEST_EQ('b', b.a.c_arr[1]);
        TEST_EQ(2.1, b.d);
        TEST_EQ('f', b.a_arr[1].c_arr[1]);

        b.a.n = 321;
        b.a_arr[1].c_arr[1] = 'y';
        dump(b, &b_pack);
        TEST_EQ(321, b_pack.a.n);
        TEST_EQ('y', b_pack.a_arr[1].c_arr[1]);
    }
    {
        abs_mem_ptr<A> a_ptr{};
        TEST_EQ(false, a_ptr.is_valid());

        const A_pack* a_pack_ptr = nullptr;
        load(&a_pack_ptr, a_ptr);
        TEST_EQ(false, !!a_ptr);

        A_pack a_pack{321, {'v', 'w'}};
        a_pack_ptr = &a_pack;
        load(&a_pack_ptr, a_ptr);
        TEST_EQ(true, !!a_ptr);
        auto& a = *a_ptr;
        TEST_EQ(321, a.n);
        TEST_EQ('w', a.c_arr[1]);
        TEST_EQ(reinterpret_cast<char*>(&a_pack) - reinterpret_cast<char*>(&a_pack_ptr), a_ptr.get());

        a.n = 555;
        a.c_arr[1] = 'e';
        dump(a_ptr, &a_pack_ptr);
        TEST_EQ(555, a_pack.n);
        TEST_EQ('e', a_pack.c_arr[1]);

        a_pack_ptr = nullptr;
        load(&a_pack_ptr, a_ptr);
        TEST_EQ(false, !!a_ptr);

        a_pack_ptr = &a_pack;
        dump(a_ptr, &a_pack_ptr);
        TEST_EQ(nullptr, a_pack_ptr);
    }
    {
        memory_offset<A, 4> a_offset{};
        A_pack_offset a_pack_offset{{}, {1, {'b', 'c'}}};

        load(&a_pack_offset, a_offset);
        A& a = a_offset.get();
        TEST_EQ(1, a.n);
        TEST_EQ('c', a.c_arr[1]);

        a.n = 567;
        a.c_arr[1] = 'r';
        dump(a_offset, &a_pack_offset);
        TEST_EQ(567, a_pack_offset.a_pack.n);
        TEST_EQ('r', a_pack_offset.a_pack.c_arr[1]);
    }
    {
        C c{};
        C_pack c_pack{};
        c_pack.a_ptr = &c_pack.a;
        c_pack.a.n = 321;
        c_pack.a.c_arr[1] = 'y';

        load(&c_pack, c);
        TEST_EQ(true, !!c.a_ptr);
        A& a1 = *c.a_ptr;
        A& a2 = c.a_offset.get();
        TEST_EQ('y', a1.c_arr[1]);
        TEST_EQ('y', a2.c_arr[1]);

        c.a_ptr.reset();
        a2.n = 456;
        a2.c_arr[1] = 'z';
        dump(c, &c_pack);
        TEST_EQ(nullptr, c_pack.a_ptr);
        TEST_EQ(456, c_pack.a.n);
        TEST_EQ('z', c_pack.a.c_arr[1]);
    }
    {
        D d{};
        D_pack d_pack{};
        d_pack.n_ptr = d_pack.n_arr;
        d_pack.n_arr[0] = 1;
        d_pack.n_arr[1] = 2;

        load(&d_pack, d);
        TEST_EQ(true, !!d.offset_ptr);
        TEST_EQ(true, !!d.ptr_offset.get());
        TEST_EQ(2, d.offset_ptr->get());
        TEST_EQ(1, *d.ptr_offset.get());

        d.offset_ptr.reset(8, d.offset_ptr.get() + 4);
        d.ptr_offset.get().reset(4);
        dump(d, &d_pack);
        TEST_EQ(&d_pack.n_arr[1], d_pack.n_ptr);
        TEST_EQ(4, d_pack.n_arr[0]);
        TEST_EQ(2, d_pack.n_arr[1]);
        TEST_EQ(8, d_pack.n_arr[2]);
    }
    {
        E e{};
        E_pack e_pack{};
        e_pack.a_ptr = e_pack.a_arr;
        e_pack.a_arr[0].n = 1;
        e_pack.a_arr[1].n = 2;

        load(&e_pack, e);
        TEST_EQ(true, !!e.arr_ptr);
        auto& a_arr1 = *e.arr_ptr;
        TEST_EQ(1, a_arr1[0].n);
        TEST_EQ(2, a_arr1[1].n);
        auto& a_arr2 = e.arr_offset.get();
        TEST_EQ(1, a_arr2[0].n);
        TEST_EQ(2, a_arr2[1].n);

        a_arr2[0].n = 5;
        a_arr2[1].n = 10;
        e.arr_ptr.reset(*e.arr_ptr, e.arr_ptr.get() + 8);
        dump(e, &e_pack);
        TEST_EQ(&e_pack.a_arr[2], e_pack.a_ptr);
        TEST_EQ(5, e_pack.a_arr[0].n);
        TEST_EQ(10, e_pack.a_arr[1].n);
        TEST_EQ(1, e_pack.a_arr[2].n);
        TEST_EQ(2, e_pack.a_arr[3].n);
    }
    {
        F f{};
        F_pack f_pack[4] = {};
        f_pack[0].f_ptr[0] = &f_pack[1];
        f_pack[0].f_ptr[1] = &f_pack[2];
        f_pack[0].a_arr[0].n = 1;
        f_pack[0].a_arr[1].n = 11;
        f_pack[1].a_arr[0].n = 2;
        f_pack[1].a_arr[1].n = 22;
        f_pack[2].a_arr[0].n = 3;
        f_pack[2].a_arr[1].n = 33;
        f_pack[3].a_arr[0].n = 4;
        f_pack[3].a_arr[1].n = 44;

        load(&f_pack[0], f);
        TEST_EQ(true, !!f.ptr_arr[0]);
        TEST_EQ(true, !!f.ptr_arr[1]);
        TEST_EQ(false, !!f.ptr_arr[0]->ptr_arr[0]);
        TEST_EQ(false, !!f.ptr_arr[1]->ptr_arr[0]);
        TEST_EQ(11, f.offset_arr.get()[1].n);
        TEST_EQ(2, f.ptr_arr[0]->offset_arr.get()[0].n);
        TEST_EQ(33, f.ptr_arr[1]->offset_arr.get()[1].n);

        f.offset_arr.get()[1].n = 111;
        f.ptr_arr[0].reset();
        f.ptr_arr[1].reset(*f.ptr_arr[1], f.ptr_arr[1].get() + sizeof(F_pack));
        f.ptr_arr[1]->offset_arr.get()[1].n = 99;
        {
            // copy ctor
            F f_copy = f;
            dump(f_copy, &f_pack[0]);
            TEST_EQ(111, f_pack[0].a_arr[1].n);
            TEST_EQ(nullptr, f_pack[0].f_ptr[0]);
            TEST_EQ(99, f_pack[0].f_ptr[1]->a_arr[1].n);
            TEST_EQ(22, f_pack[1].a_arr[1].n);
            TEST_EQ(33, f_pack[2].a_arr[1].n);
            TEST_EQ(3, f_pack[3].a_arr[0].n);
            TEST_EQ(99, f_pack[3].a_arr[1].n);
        }
        {
            // dump const value
            const F f_copy = f;
            dump(f_copy, &f_pack[0]);
            TEST_EQ(111, f_pack[0].a_arr[1].n);
            TEST_EQ(nullptr, f_pack[0].f_ptr[0]);
            TEST_EQ(99, f_pack[0].f_ptr[1]->a_arr[1].n);
            TEST_EQ(22, f_pack[1].a_arr[1].n);
            TEST_EQ(33, f_pack[2].a_arr[1].n);
            TEST_EQ(3, f_pack[3].a_arr[0].n);
            TEST_EQ(99, f_pack[3].a_arr[1].n);
        }
        {
            // move ctor
            F _f = f;
            F f_copy = std::move(_f);
            dump(f_copy, &f_pack[0]);
            TEST_EQ(111, f_pack[0].a_arr[1].n);
            TEST_EQ(nullptr, f_pack[0].f_ptr[0]);
            TEST_EQ(99, f_pack[0].f_ptr[1]->a_arr[1].n);
            TEST_EQ(22, f_pack[1].a_arr[1].n);
            TEST_EQ(33, f_pack[2].a_arr[1].n);
            TEST_EQ(3, f_pack[3].a_arr[0].n);
            TEST_EQ(99, f_pack[3].a_arr[1].n);
        }
        {
            // operator=(&)
            F f_copy;
            f_copy = f;
            dump(f_copy, &f_pack[0]);
            TEST_EQ(111, f_pack[0].a_arr[1].n);
            TEST_EQ(nullptr, f_pack[0].f_ptr[0]);
            TEST_EQ(99, f_pack[0].f_ptr[1]->a_arr[1].n);
            TEST_EQ(22, f_pack[1].a_arr[1].n);
            TEST_EQ(33, f_pack[2].a_arr[1].n);
            TEST_EQ(3, f_pack[3].a_arr[0].n);
            TEST_EQ(99, f_pack[3].a_arr[1].n);
        }
        {
            // operator=(&&)
            F f_copy;
            F _f = f;
            f_copy = std::move(_f);
            dump(f_copy, &f_pack[0]);
            TEST_EQ(111, f_pack[0].a_arr[1].n);
            TEST_EQ(nullptr, f_pack[0].f_ptr[0]);
            TEST_EQ(99, f_pack[0].f_ptr[1]->a_arr[1].n);
            TEST_EQ(22, f_pack[1].a_arr[1].n);
            TEST_EQ(33, f_pack[2].a_arr[1].n);
            TEST_EQ(3, f_pack[3].a_arr[0].n);
            TEST_EQ(99, f_pack[3].a_arr[1].n);
        }
    }

    return 0;
}
