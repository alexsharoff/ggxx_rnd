#include "test.h"

#include "fiber_mgmt.h"

#include <Windows.h>

#include <unordered_map>


using fiber_mgmt::fiber_service;
using fiber_mgmt::fiber_state;

struct fiber_arg
{
    bool done = false;
    LPVOID other_fiber = nullptr;
    size_t counter = 0;
};

__declspec(noinline) void do_work(fiber_arg& arg, size_t test)
{
    size_t pad[0x100];
    std::fill_n(pad, 0x100, test);
    ++arg.counter;
    ::SwitchToFiber(arg.other_fiber);
    if (pad[0xff] != test)
        std::exit(-4);
}

void WINAPI fiber_f(LPVOID arg_)
{
    const size_t size = 0x800;
    uint32_t pad1[size] = { 0 };
    fiber_arg* arg = static_cast<fiber_arg*>(arg_);
    uint32_t pad2[size] = { 0 };
    size_t n = 3;
    arg->done = false;
    while(n--)
    {
        pad1[n] = 1;
        do_work(*arg, rand());
        pad2[n] = 1;
    }
    size_t i = 0;
    while (i < size)
    {
        if (pad1[i] != pad2[i])
            std::exit(-1);
        if (!pad1[i])
            break;
        ++i;
    }
    if (i != 3)
        std::exit(-2);
    arg->done = true;
    ::SwitchToFiber(arg->other_fiber);
    // can't reach here under normal conditions
    std::exit(-3);
}

int test1(LPVOID main_fiber)
{
    fiber_arg arg = { false, main_fiber };
    const auto fiber = ::CreateFiber(0, fiber_f, &arg);
    while (!arg.done)
    {
        ::SwitchToFiber(fiber);
    }
    ::DeleteFiber(fiber);
    TEST_EQ(arg.counter, 3);

    return 0;
}

int test2(LPVOID main_fiber, fiber_service::ptr_t service)
{
    fiber_arg arg = { false, main_fiber };
    const auto fiber = ::CreateFiber(0, fiber_f, &arg);
    fiber_state state;
    service->load(fiber, state);
    while (!arg.done)
    {
        ::SwitchToFiber(fiber);
    }
    TEST_EQ(arg.counter, 3);

    // restore previous state
    service->restore(state);
    arg.done = false;
    // continue fiber execution from previous state
    while (!arg.done)
    {
        ::SwitchToFiber(fiber);
    }
    // reference counter wasn't free'd, so DeleteFiber does nothing here
    ::DeleteFiber(fiber);
    TEST_EQ(arg.counter, 6);

    return 0;
}

int test3(LPVOID main_fiber, size_t n, fiber_service::ptr_t service)
{
    std::vector<fiber_arg> fiber_args(n, { false, main_fiber });
    std::vector<LPVOID> fibers;
    for (size_t i = 0; i < n; ++i)
        fibers.push_back(::CreateFiber(0, fiber_f, &fiber_args[i]));
    std::unordered_map<LPVOID, fiber_state> state_map;
    while (true)
    {
        bool all_done = true;
        for (size_t i = 0; i < n; ++i)
        {
            const auto fiber = fibers[i];
            ::SwitchToFiber(fiber);
            if (state_map.find(fiber) == state_map.end())
            {
                fiber_state state;
                service->load(fiber, state);
                state_map[fiber] = state;
            }
            if (!fiber_args[i].done)
                all_done = false;
        }
        if (all_done)
            break;
    }
    for (const auto& arg : fiber_args)
    {
        TEST_EQ(arg.counter, 3);
    }

    // restore previous state
    for (const auto fiber : fibers)
    {
        service->restore(state_map[fiber]);
    }
    for (auto& arg : fiber_args)
        arg.done = false;
    // continue fiber execution from previous state
    while (true)
    {
        bool all_done = true;
        for (size_t i = 0; i < n; ++i)
        {
            const auto fiber = fibers[i];
            ::SwitchToFiber(fiber);
            if (!fiber_args[i].done)
                all_done = false;
        }
        if (all_done)
            break;
    }
    for (const auto& arg : fiber_args)
    {
        TEST_EQ(arg.counter, 5);
    }
    state_map.clear();
    for (const auto fiber : fibers)
    {
        // ref counter was free'd, fiber will be destroyed now
        ::DeleteFiber(fiber);
    }

    return 0;
}

int test4(LPVOID main_fiber)
{
    fiber_arg arg = { false, main_fiber };
    const auto fiber = ::CreateFiber(0, fiber_f, &arg);
    auto service = fiber_service::start();
    fiber_state state;
    service->transfer_ownership(fiber);
    service->load(fiber, state);
    while (!arg.done)
    {
        ::SwitchToFiber(fiber);
    }
    TEST_EQ(arg.counter, 3);

    arg.done = false;

    // restore previous state
    service->restore(state);
    // continue fiber execution from previous state
    while (!arg.done)
    {
        ::SwitchToFiber(fiber);
    }
    ::DeleteFiber(fiber);
    TEST_EQ(arg.counter, 6);

    return 0;
}

int test5(LPVOID main_fiber)
{
    fiber_arg arg = { false, main_fiber };
    const auto fiber = ::CreateFiber(0, fiber_f, &arg);
    fiber_service::ptr_t service;
    fiber_state state;
    while (!arg.done)
    {
        ::SwitchToFiber(fiber);
        if (!service)
        {
            service = fiber_service::start();
            service->transfer_ownership(fiber);
            service->load(fiber, state);
        }
    }
    TEST_EQ(arg.counter, 3);

    // restore previous state
    service->restore(state);
    arg.done = false;
    // continue fiber execution from previous state
    while (!arg.done)
    {
        ::SwitchToFiber(fiber);
    }
    ::DeleteFiber(fiber);
    TEST_EQ(arg.counter, 5);

    return 0;
}

int test6(LPVOID main_fiber, fiber_service::ptr_t service)
{
    fiber_arg arg = { false, main_fiber };
    const auto fiber = ::CreateFiber(0, fiber_f, &arg);
    fiber_state state;
    service->load(fiber, state);
    while (!arg.done)
    {
        ::SwitchToFiber(fiber);
    }
    TEST_EQ(arg.counter, 3);

    // reference counter on fiber_state wasn't free'd, so DeleteFiber does nothing here
    ::DeleteFiber(fiber);

    // restore previous state
    service->restore(state);

    fiber_state state2;
    arg.done = false;
    // continue fiber execution from previous state
    while (!arg.done)
    {
        ::SwitchToFiber(fiber);
        // update state after DeleteFiber
        if (!state2.shared)
        {
            service->load(fiber, state2);
        }
    }

    // restore previous state
    service->restore(state2);
    arg.done = false;
    // continue fiber execution from previous state
    while (!arg.done)
    {
        ::SwitchToFiber(fiber);
    }

    TEST_EQ(arg.counter, 8);

    return 0;
}

int main()
{
    const auto main_fiber = ::ConvertThreadToFiber(0);
    if (test1(main_fiber))
        return 1;

    {
        auto service = fiber_service::start();
        for (int i = 0; i < 3; ++i)
        {
            if (test1(main_fiber))
                return 2;
        }
    }

    {
        auto service = fiber_service::start();
        // init/shutdown cycle without work
    }

    {
        auto service = fiber_service::start();
        if (test2(main_fiber, service))
            return 3;
    }

    {
        auto service = fiber_service::start();
        if (test3(main_fiber, 5, service))
            return 4;
    }

    {
        auto service = fiber_service::start();
        for (int i = 0; i < 3; ++i)
        {
            if (test2(main_fiber, service))
                return 5;

            if (test3(main_fiber, 5, service))
                return 6;
        }
    }

    if (test4(main_fiber))
        return 7;

    if (test5(main_fiber))
        return 8;

    {
        auto service = fiber_service::start();
        if (test6(main_fiber, service))
            return 9;
    }

    return 0;
}
