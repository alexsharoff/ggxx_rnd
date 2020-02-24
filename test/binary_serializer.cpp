#include "binary_serializer.h"
#include "memory_dump.h"
#include "mini_reflection.h"

#include "test.h"

using binary_serializer::serialize;
using binary_serializer::deserialize;
using binary_serializer::max_serialized_size;
using memory_dump::abs_mem_ptr;
using memory_dump::memory_offset;
using mini_reflection::reflect;
using mini_reflection::member_tuple;
using mini_reflection::sizeof_noalign;

struct empty_t {};
template<>
struct reflect<empty_t>
{
    constexpr static auto members = member_tuple();
};

struct A
{
    uint32_t n1;
    char c_arr[2];
};
template<>
struct reflect<A>
{
    constexpr static auto members = member_tuple(
        &A::n1,
        &A::c_arr
    );
};

struct B
{
    uint32_t n1;
    abs_mem_ptr<A> a_ptr[2];
};
template<>
struct reflect<B>
{
    constexpr static auto members = member_tuple(
        &B::n1,
        &B::a_ptr
    );
};

struct C
{
    memory_offset<abs_mem_ptr<B[2]>, 0x2a2> b_arr_ptr;
    memory_offset<uint64_t, 0x132> n1;
};
template<>
struct reflect<C>
{
    constexpr static auto members = member_tuple(
        &C::n1,
        &C::b_arr_ptr
    );
};


int main()
{
    using std::tie;
    {
        char memory[4];
        uint32_t value = 0x38383838;
        auto [success, offset] = serialize(value, memory, 0);
        TEST_EQ(false, success);
        TEST_EQ(0, offset);
        tie(success, offset) = serialize(value, memory, 2);
        TEST_EQ(false, success);
        TEST_EQ(0, offset);
        tie(success, offset) = serialize(value, memory, 4);
        TEST_EQ(true, success);
        TEST_EQ(4, offset);
        TEST_EQ('8', memory[0]);
        TEST_EQ('8', memory[3]);

        for (auto& c : memory)
            c = -1;
        value = 0;
        tie(success, offset) = deserialize(value, memory, 0);
        TEST_EQ(false, success);
        TEST_EQ(0, offset);
        tie(success, offset) = deserialize(value, memory, 2);
        TEST_EQ(false, success);
        TEST_EQ(0, offset);
        tie(success, offset) = deserialize(value, memory, 4);
        TEST_EQ(true, success);
        TEST_EQ(4, offset);
        TEST_EQ(0xffffffff, value);
    }
    {
        char memory[1];
        empty_t value;
        auto [success, offset] = serialize(value, memory, 1);
        TEST_EQ(true, success);
        TEST_EQ(0, offset);
        tie(success, offset) = deserialize(value, memory, 1);
        TEST_EQ(true, success);
        TEST_EQ(0, offset);
    }
    {
        const size_t size_max = max_serialized_size<C>::value;
        const size_t size_min = sizeof_noalign<C>::value;

        char memory[size_max];
        C value = {};
        value.n1 = 123;
        auto [success, offset] = serialize(value, memory, 0);
        TEST_EQ(false, success);
        TEST_EQ(0, offset);
        tie(success, offset) = serialize(value, memory, size_min - 1);
        TEST_EQ(false, success);
        tie(success, offset) = serialize(value, memory, size_min);
        TEST_EQ(true, success);
        TEST_EQ(size_min, offset);

        value.n1 = 0;
        tie(success, offset) = deserialize(value, memory, 0);
        TEST_EQ(false, success);
        TEST_EQ(0, offset);
        tie(success, offset) = deserialize(value, memory, size_min - 1);
        TEST_EQ(false, success);
        tie(success, offset) = deserialize(value, memory, size_min);
        TEST_EQ(true, success);
        TEST_EQ(size_min, offset);
        TEST_EQ(123, value.n1);

        value.n1 = 1;
        value.b_arr_ptr.get().reset({
                B{
                    2,
                    {
                        {{3, {'a', 'b'}}, 0x111},
                        {{4, {'c', 'd'}}, 0x222}
                    }
                },
                B{
                    5,
                    {
                        {{6, {'e', 'f'}}, 0x333},
                        {{7, {'g', 'h'}}, 0x444}
                    }
                }
            },
            0x555
        );
        tie(success, offset) = serialize(value, memory, size_max - 1);
        TEST_EQ(false, success);
        tie(success, offset) = serialize(value, memory, size_max);
        TEST_EQ(true, success);
        TEST_EQ(size_max, offset);

        value.n1 = 0;
        value.b_arr_ptr.get().reset();
        tie(success, offset) = deserialize(value, memory, size_max - 1);
        TEST_EQ(false, success);
        tie(success, offset) = deserialize(value, memory, size_max);
        TEST_EQ(true, success);
        TEST_EQ(size_max, offset);

        TEST_EQ(1, value.n1);
        TEST_EQ(0x555, value.b_arr_ptr.get().get());
        B& b = (*value.b_arr_ptr.get())[0];
        TEST_EQ(2, b.n1);
        TEST_EQ(0x111, b.a_ptr[0].get());
        TEST_EQ(3, b.a_ptr[0]->n1);
        TEST_EQ('b', b.a_ptr[0]->c_arr[1]);
        TEST_EQ(0x222, b.a_ptr[1].get());
        TEST_EQ(4, b.a_ptr[1]->n1);
        TEST_EQ('d', b.a_ptr[1]->c_arr[1]);
        b = (*value.b_arr_ptr.get())[1];
        TEST_EQ(5, b.n1);
        TEST_EQ(0x333, b.a_ptr[0].get());
        TEST_EQ(6, b.a_ptr[0]->n1);
        TEST_EQ('f', b.a_ptr[0]->c_arr[1]);
        TEST_EQ(0x444, b.a_ptr[1].get());
        TEST_EQ(7, b.a_ptr[1]->n1);
        TEST_EQ('h', b.a_ptr[1]->c_arr[1]);
    }

    return 0;
}
