#include "test.h"

#include "fiber_mgmt.h"

#include <Windows.h>

#include <unordered_map>


struct fiber_arg
{
    bool done = false;
    LPVOID other_fiber = nullptr;
    size_t counter = 0;
};

__declspec(noinline) void do_work(fiber_arg& arg)
{
    ++arg.counter;
    ::SwitchToFiber(arg.other_fiber);
}

void WINAPI fiber_f(LPVOID arg_)
{
    fiber_arg* arg = static_cast<fiber_arg*>(arg_);
    arg->done = false;
    size_t n = 3;
    while(n--)
    {
        do_work(*arg);
    }
    arg->done = true;
    ::SwitchToFiber(arg->other_fiber);
    // can't reach here under normal conditions
    std::exit(-1);
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

int test2(LPVOID main_fiber)
{
    fiber_arg arg = { false, main_fiber };
    const auto fiber = ::CreateFiber(0, fiber_f, &arg);
    fiber_mgmt::fiber_state state;
    fiber_mgmt::load_state(fiber, state);
    while (!arg.done)
    {
        ::SwitchToFiber(fiber);
    }
    TEST_EQ(arg.counter, 3);

    // restore previous state
    fiber_mgmt::dump_state(state);
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

int test3(LPVOID main_fiber, size_t n)
{
    std::vector<fiber_arg> fiber_args(n, { false, main_fiber });
    std::vector<LPVOID> fibers;
    for (size_t i = 0; i < n; ++i)
        fibers.push_back(::CreateFiber(0, fiber_f, &fiber_args[i]));
    std::unordered_map<LPVOID, fiber_mgmt::fiber_state> state_map;
    while (true)
    {
        bool all_done = true;
        for (size_t i = 0; i < n; ++i)
        {
            const auto fiber = fibers[i];
            ::SwitchToFiber(fiber);
            if (state_map.find(fiber) == state_map.end())
            {
                fiber_mgmt::fiber_state state;
                fiber_mgmt::load_state(fiber, state);
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
        fiber_mgmt::dump_state(state_map[fiber]);
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
    fiber_mgmt::fiber_state state;
    fiber_mgmt::init();
    fiber_mgmt::transfer_ownership(fiber);
    fiber_mgmt::load_state(fiber, state);
    while (!arg.done)
    {
        ::SwitchToFiber(fiber);
    }
    TEST_EQ(arg.counter, 3);

    // restore previous state
    fiber_mgmt::dump_state(state);
    arg.done = false;
    // continue fiber execution from previous state
    while (!arg.done)
    {
        ::SwitchToFiber(fiber);
    }
    ::DeleteFiber(fiber);
    TEST_EQ(arg.counter, 6);

    fiber_mgmt::shutdown();

    return 0;
}

int test5(LPVOID main_fiber)
{
    fiber_arg arg = { false, main_fiber };
    const auto fiber = ::CreateFiber(0, fiber_f, &arg);
    fiber_mgmt::fiber_state state;
    bool init = true;
    while (!arg.done)
    {
        ::SwitchToFiber(fiber);
        if (init)
        {
            fiber_mgmt::init();
            fiber_mgmt::transfer_ownership(fiber);
            fiber_mgmt::load_state(fiber, state);
            init = false;
        }
    }
    TEST_EQ(arg.counter, 3);

    // restore previous state
    fiber_mgmt::dump_state(state);
    arg.done = false;
    // continue fiber execution from previous state
    while (!arg.done)
    {
        ::SwitchToFiber(fiber);
    }
    ::DeleteFiber(fiber);
    TEST_EQ(arg.counter, 5);

    fiber_mgmt::shutdown();

    return 0;
}

int main()
{
    const auto main_fiber = ::ConvertThreadToFiber(0);
    if (test1(main_fiber))
        return 1;

    fiber_mgmt::init();
    for (int i = 0; i < 3; ++i)
    {
        if (test1(main_fiber))
            return 2;
    }
    fiber_mgmt::shutdown();

    fiber_mgmt::init();
    // init/shutdown cycle without work
    fiber_mgmt::shutdown();

    fiber_mgmt::init();
    if (test2(main_fiber))
        return 3;
    fiber_mgmt::shutdown();

    fiber_mgmt::init();
    if (test3(main_fiber, 5))
        return 4;
    fiber_mgmt::shutdown();

    fiber_mgmt::init();
    for (int i = 0; i < 3; ++i)
    {
        if (test2(main_fiber))
            return 5;

        if (test3(main_fiber, 5))
            return 6;
    }
    fiber_mgmt::shutdown();

    if (test4(main_fiber))
        return 7;

    if (test5(main_fiber))
        return 8;

    return 0;
}
