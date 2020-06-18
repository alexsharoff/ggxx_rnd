#include "mini_reflection.h"

#include <cstdint>
#include <iostream>
#include <sstream>

#include "test.h"

using mini_reflection::reflect;
using mini_reflection::member_tuple;
using mini_reflection::member_descr;
using mini_reflection::self;
using mini_reflection::array;
using mini_reflection::for_each_member;
using mini_reflection::sizeof_noalign;

typedef int (*func_t)();
int func1(){ return 0; }

struct empty {};

struct A
{
    char c;
};

struct B
{
    char c;
};

template<>
struct reflect<B>
{
    constexpr static auto members = member_tuple(
        &B::c);
};

struct C
{
    uint32_t n_arr[3];
    bool b;
};

template<>
struct reflect<C>
{
    constexpr static auto members = member_tuple(
        &C::b, "b",
        &C::n_arr, "n_arr");
};

enum Enum : int8_t
{
    Enum1,
    Enum2,
    Enum3
};

struct D
{
    Enum e;
    func_t f_ptr;
    void func() {}
};

template<>
struct reflect<D>
{
    constexpr static auto members = member_tuple(
        &D::e, "e",
        &D::f_ptr, "f_ptr");
};

struct E
{
    B b;
    uint64_t n_arr[3];
};

template<>
struct reflect<E>
{
    constexpr static auto members = member_tuple(
        &E::b, "b",
        &E::n_arr, "n_arr");
};

struct F : E
{
    float f_arr[4];
};

template<>
struct reflect<F>
{
    constexpr static auto members = member_tuple(
        &E::b, "b",
        &E::n_arr, "n_arr",
        &F::f_arr, "f_arr");
};

struct G : F
{
    bool b_arr[3];
};

template<>
struct reflect<G>
{
    constexpr static auto members = member_tuple(
        reflect<F>::members,
        &G::b_arr, "b_arr");
};

struct H {};

template<>
struct reflect<H>
{
    constexpr static auto members = member_tuple();
};

struct I
{
    B b_arr[3];
    float f;
};

template<>
struct reflect<I>
{
    constexpr static auto members = member_tuple(
        &I::b_arr, "b_arr",
        &I::f, "f");
};

struct J
{
    char c1;
    E e_arr[2];
    char c2;
};

template<>
struct reflect<J>
{
    constexpr static auto members = member_tuple(
        &J::c1, "c1",
        &J::e_arr, "e_arr",
        &J::c2, "c2");
};

struct K
{
    char c1;
    G g_arr[2];
    char c2;
};

template<>
struct reflect<K>
{
    constexpr static auto members = member_tuple(
        &K::c1, "c1",
        &K::g_arr, "g_arr",
        &K::c2, "c2");
};

static_assert(sizeof_noalign<A>::value == 1);
static_assert(sizeof_noalign<A&>::value == 1);
static_assert(sizeof_noalign<A&&>::value == 1);
static_assert(sizeof_noalign<const A>::value == 1);
static_assert(sizeof_noalign<const A&>::value == 1);
static_assert(sizeof_noalign<const A&&>::value == 1);
static_assert(sizeof_noalign<volatile A>::value == 1);
static_assert(sizeof_noalign<volatile A&>::value == 1);
static_assert(sizeof_noalign<volatile A&&>::value == 1);
static_assert(sizeof_noalign<const volatile A>::value == 1);
static_assert(sizeof_noalign<const volatile A&>::value == 1);
static_assert(sizeof_noalign<const volatile A&&>::value == 1);

static_assert(sizeof_noalign<B>::value == 1);
static_assert(sizeof_noalign<B&>::value == 1);
static_assert(sizeof_noalign<B&&>::value == 1);
static_assert(sizeof_noalign<const B>::value == 1);
static_assert(sizeof_noalign<const B&>::value == 1);
static_assert(sizeof_noalign<const B&&>::value == 1);
static_assert(sizeof_noalign<volatile B>::value == 1);
static_assert(sizeof_noalign<volatile B&>::value == 1);
static_assert(sizeof_noalign<volatile B&&>::value == 1);
static_assert(sizeof_noalign<const volatile B>::value == 1);
static_assert(sizeof_noalign<const volatile B&>::value == 1);
static_assert(sizeof_noalign<const volatile B&&>::value == 1);

static_assert(sizeof_noalign<C>::value == 13);
static_assert(sizeof_noalign<D>::value == 1 + sizeof(func_t));
static_assert(sizeof_noalign<E>::value == 25);
static_assert(sizeof_noalign<F>::value == 41);
static_assert(sizeof_noalign<G>::value == 44);
static_assert(sizeof_noalign<H>::value == 0);
static_assert(sizeof_noalign<I>::value == 7);
static_assert(sizeof_noalign<J>::value == 52);
static_assert(sizeof_noalign<K>::value == 90);

static_assert(sizeof_noalign<uint32_t>::value == 4);
static_assert(sizeof_noalign<uint32_t&>::value == 4);
static_assert(sizeof_noalign<uint32_t&&>::value == 4);
static_assert(sizeof_noalign<const uint32_t>::value == 4);
static_assert(sizeof_noalign<const uint32_t&>::value == 4);
static_assert(sizeof_noalign<const uint32_t&&>::value == 4);
static_assert(sizeof_noalign<volatile uint32_t>::value == 4);
static_assert(sizeof_noalign<volatile uint32_t&>::value == 4);
static_assert(sizeof_noalign<volatile uint32_t&&>::value == 4);
static_assert(sizeof_noalign<const volatile uint32_t>::value == 4);
static_assert(sizeof_noalign<const volatile uint32_t&>::value == 4);
static_assert(sizeof_noalign<const volatile uint32_t&&>::value == 4);

using c_arr = char[7];
static_assert(sizeof_noalign<c_arr>::value == 7);
static_assert(sizeof_noalign<c_arr&>::value == 7);
static_assert(sizeof_noalign<c_arr&&>::value == 7);
static_assert(sizeof_noalign<const c_arr>::value == 7);
static_assert(sizeof_noalign<const c_arr&>::value == 7);
static_assert(sizeof_noalign<const c_arr&&>::value == 7);
static_assert(sizeof_noalign<volatile c_arr>::value == 7);
static_assert(sizeof_noalign<volatile c_arr&>::value == 7);
static_assert(sizeof_noalign<volatile c_arr&&>::value == 7);
static_assert(sizeof_noalign<const volatile c_arr>::value == 7);
static_assert(sizeof_noalign<const volatile c_arr&>::value == 7);
static_assert(sizeof_noalign<const volatile c_arr&&>::value == 7);

static_assert(sizeof_noalign<empty>::value == 1);
static_assert(sizeof_noalign<empty&>::value == 1);
static_assert(sizeof_noalign<empty&&>::value == 1);
static_assert(sizeof_noalign<const empty>::value == 1);
static_assert(sizeof_noalign<const empty&>::value == 1);
static_assert(sizeof_noalign<const empty&&>::value == 1);
static_assert(sizeof_noalign<volatile empty>::value == 1);
static_assert(sizeof_noalign<volatile empty&>::value == 1);
static_assert(sizeof_noalign<volatile empty&&>::value == 1);
static_assert(sizeof_noalign<const volatile empty>::value == 1);
static_assert(sizeof_noalign<const volatile empty&>::value == 1);
static_assert(sizeof_noalign<const volatile empty&&>::value == 1);

struct simple_printer
{
    template<class T, class FuncT, class... Args>
    static void for_each_member_idx(T&& o, FuncT f, Args&&... args)
    {
        std::apply(
            [&](const auto&... m)
            {
                const size_t last = sizeof...(m) - 1;
                size_t idx = 0;
                (f(std::forward<T>(o), m, idx++, last, std::forward<Args>(args)...), ...);
            },
            reflect<T>::members
        );
    }

    template<class T>
    void operator()(const T& o, const self<T>&, size_t, size_t, std::ostream& os) const
    {
        os << o;
    }

    template<class T>
    void operator()(const volatile T&, const self<T>&, size_t, size_t, std::ostream& os) const
    {
        os << "volatile";
    }

    template<class T, size_t N>
    void operator()(const T (&arr)[N], const array<T, N>&, size_t, size_t, std::ostream& os) const
    {
        for (size_t i = 0; i < N; ++i)
        {
            for_each_member_idx(arr[i], *this, os);
            if (i + 1 < N)
                os << ',';
        }
    }

    template<class T, size_t N>
    void operator()(const volatile T (&/* arr */)[N], const array<T, N>&, size_t, size_t, std::ostream& os) const
    {
        os << "volatile";
    }

    void operator()(const A&, const self<A>&, size_t, size_t, std::ostream& os) const
    {
        os << "A";
    }

    void operator()(const volatile A&, const self<A>&, size_t, size_t, std::ostream& os) const
    {
        os << "volatile";
    }

    template<class T, class BaseT, class MemberT>
    void operator()(
        const T& o, const member_descr<BaseT, MemberT>& m, size_t idx, size_t last,
        std::ostream& os) const
    {
        if (!m.name.empty())
            os << m.name << ':';
        for_each_member_idx(o.*m.member, *this, os);
        if (last != idx)
            os << ' ';
    }

    template<class T, class BaseT, class MemberT>
    void operator()(
        const volatile T& /* o */, const member_descr<BaseT, MemberT>&, size_t, size_t,
        std::ostream& os) const
    {
        os << "volatile";
    }

    template<class T>
    void print(T&& val, std::ostream& os)
    {
        for_each_member_idx(val, *this, os);
    }
};

struct increment
{
    template<class IgnoreT1, class IgnoreT2, class T3>
    void operator()(IgnoreT1&&, const IgnoreT2&, T3& value) const
    {
        ++value;
    }
};

#define TEST_RECURSIVE_PRINT(value, expected) { \
    simple_printer sp; \
    std::ostringstream oss; \
    sp.print(value, oss); \
    TEST_EQ(oss.str(), expected); \
}

#define TEST_COUNT_MEMBERS(value, expected) { \
    size_t n = 0; \
    for_each_member(value, increment{}, n); \
    TEST_EQ(n, expected); \
    n = 0; \
    for_each_member(static_cast<std::add_lvalue_reference_t<decltype(value)>>(value), increment{}, n); \
    TEST_EQ(n, expected); \
    n = 0; \
    for_each_member(static_cast<std::add_rvalue_reference_t<decltype(value)>>(std::move(value)), increment{}, n); \
    TEST_EQ(n, expected); \
}

int main()
{
    {
        uint32_t value = 123;
        TEST_RECURSIVE_PRINT(value, "123");
        TEST_COUNT_MEMBERS(value, 1);
    }
    {
        const uint32_t value = 123;
        TEST_RECURSIVE_PRINT(value, "123");
        TEST_COUNT_MEMBERS(value, 1);
    }
    {
        volatile const uint32_t value = 123;
        TEST_RECURSIVE_PRINT(value, "volatile");
        TEST_COUNT_MEMBERS(value, 1);
    }
    {
        volatile uint32_t value = 123;
        TEST_RECURSIVE_PRINT(value, "volatile");
        TEST_COUNT_MEMBERS(value, 1);
    }
    {
        uint32_t value[] = {1, 2, 3};
        TEST_RECURSIVE_PRINT(value, "1,2,3");
        TEST_COUNT_MEMBERS(value, 1);
    }
    {
        const uint32_t value[] = {1, 2, 3};
        TEST_RECURSIVE_PRINT(value, "1,2,3");
        TEST_COUNT_MEMBERS(value, 1);
    }
    {
        volatile uint32_t value[] = {1, 2, 3};
        TEST_RECURSIVE_PRINT(value, "volatile");
        TEST_COUNT_MEMBERS(value, 1);
    }
    {
        volatile const uint32_t value[] = {1, 2, 3};
        TEST_RECURSIVE_PRINT(value, "volatile");
        TEST_COUNT_MEMBERS(value, 1);
    }
    {
        A value{};
        TEST_RECURSIVE_PRINT(value, "A");
        TEST_COUNT_MEMBERS(value, 1);
    }
    {
        B value{'b'};
        TEST_RECURSIVE_PRINT(value, "b");
        TEST_COUNT_MEMBERS(value, 1);
    }
    {
        const B value{'b'};
        TEST_RECURSIVE_PRINT(value, "b");
        TEST_COUNT_MEMBERS(value, 1);
    }
    {
        volatile B value{'b'};
        TEST_RECURSIVE_PRINT(value, "volatile");
        TEST_COUNT_MEMBERS(value, 1);
    }
    {
        volatile const B value{'b'};
        TEST_RECURSIVE_PRINT(value, "volatile");
        TEST_COUNT_MEMBERS(value, 1);
    }
    {
        C value{{1,2,3}, true};
        TEST_RECURSIVE_PRINT(value, "b:1 n_arr:1,2,3");
        TEST_COUNT_MEMBERS(value, 2);
    }
    {
        D value{Enum3, func1};
        std::ostringstream tmp;
        tmp << &func1;
        TEST_RECURSIVE_PRINT(value, "e:\x2 f_ptr:" + tmp.str());
        TEST_COUNT_MEMBERS(value, 2);
    }
    {
        E value{{'e'}, {3, 2, 1}};
        TEST_RECURSIVE_PRINT(value, "b:e n_arr:3,2,1");
        TEST_COUNT_MEMBERS(value, 2);
    }
    {
        F value{{{'f'}, {0, 5, 65535}}, {5, 6, 7, 8}};
        TEST_RECURSIVE_PRINT(value, "b:f n_arr:0,5,65535 f_arr:5,6,7,8");
        TEST_COUNT_MEMBERS(value, 3);
    }
    {
        G value{{{{'g'}, {3, 2, 1}}, {-1, -2, -3, -4}}, {true, false, true}};
        TEST_RECURSIVE_PRINT(value, "b:g n_arr:3,2,1 f_arr:-1,-2,-3,-4 b_arr:1,0,1");
        TEST_COUNT_MEMBERS(value, 4);
    }
    {
        H value{};
        TEST_RECURSIVE_PRINT(value, "");
        TEST_COUNT_MEMBERS(value, 0);
    }
    {
        I value{{{'a'}, {'b'}, {'c'}}, 555};
        TEST_RECURSIVE_PRINT(value, "b_arr:a,b,c f:555");
        TEST_COUNT_MEMBERS(value, 2);
    }
    {
        J value{'x', {{{'a'}, {1, 2, 3}}, {{'b'}, {4, 5, 6}}}, 'y'};
        TEST_RECURSIVE_PRINT(value, "c1:x e_arr:b:a n_arr:1,2,3,b:b n_arr:4,5,6 c2:y");
        TEST_COUNT_MEMBERS(value, 3);
    }
    {
        K value{
            'x',
            {
                {{{{'a'}, {1, 2, 3}}, {4, 5, 6, 7}}, {false, false, false}},
                {{{{'b'}, {8, 9, 10}}, {11, 12, 13, 14}}, {true, true, true}}
            },
            'y'
        };
        TEST_RECURSIVE_PRINT(value, "c1:x g_arr:b:a n_arr:1,2,3 f_arr:4,5,6,7 b_arr:0,0,0,b:b n_arr:8,9,10 f_arr:11,12,13,14 b_arr:1,1,1 c2:y");
        TEST_COUNT_MEMBERS(value, 3);
    }

    return 0;
}
