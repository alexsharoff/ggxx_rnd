#pragma once

#include <Windows.h>

#include <functional>
#include <memory>
#include <vector>


namespace fiber_mgmt
{

typedef VOID (WINAPI delete_fiber_func_t)(LPVOID);

struct fiber_state
{
    struct data_
    {
        uint8_t data1[0x4]; // 0
        size_t* seh_record; // 4
        uint8_t data2[0x4]; // 8
        size_t* stack_begin; // C
        uint8_t data3[0xC8]; // 10
        size_t* esp; // D8
        uint8_t data4[0x21C]; // DC
    } data; // 2F8
    // TODO: try to use static array here
    std::vector<size_t> stack;
    struct refcount_
    {
        refcount_(LPVOID f_, std::function<void(LPVOID)> d_, size_t s_)
            : fiber(f_)
            , deleter(d_)
            , stack_size(s_)
        {}
        ~refcount_()
        {
            deleter(fiber);
        }
        LPVOID fiber;
        std::function<void(LPVOID)> deleter;
        size_t stack_size;
    };
    std::shared_ptr<refcount_> refcount;
};
static_assert(sizeof(fiber_state::data_) == 0x2f8);

void init();

void shutdown();

void load_state(LPVOID fiber, fiber_state& state);

// Use this function instead of load_state, if DeleteFiber
// was previosly called for provided fiber.
// I.e. resurrecting a previosly "dead" fiber that was
// not completely deleted because it's reference counted
// fiber_state was saved in user code.
void update_state(fiber_state& state);

void dump_state(const fiber_state& state);

void transfer_ownership(LPVOID fiber);

}
