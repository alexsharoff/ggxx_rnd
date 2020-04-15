#include "fiber_mgmt.h"

#include "patch_iat.h"
#include "memory_dump.h"

#include <limits>
#include <string>
#include <unordered_map>


// 64-bit is not supported
static_assert(sizeof(size_t) == 4);


namespace fiber_mgmt
{

namespace
{

using memory_dump::load;
using memory_dump::dump_unprotected;

typedef LPVOID (WINAPI create_fiber_func_t)(SIZE_T, LPFIBER_START_ROUTINE, LPVOID);

std::unordered_map<LPVOID, std::shared_ptr<fiber_state::refcount_>> g_fiber_map_owner;
std::unordered_map<LPVOID, std::weak_ptr<fiber_state::refcount_>> g_fiber_map_weak;

size_t get_stack_size(LPVOID fiber)
{
    fiber_state::data_ data;
    load(fiber, data);
    size_t* end = data.esp;
    if ((size_t)data.seh_record != std::numeric_limits<size_t>::max())
    {
        // If seh_record is valid, it means that SwitchToFiber
        // was called at least once for current fiber.
        // In this case ESP no longer points to stack bottom.
        // 
        // Let's use SEH record pointer instead of ESP.
        // It's always near the bottom of the stack and
        // it's highly inlikely that anything below it
        // is going to be overwritten during normal fiber
        // execution.
        end = data.seh_record;
    }
    return end - data.stack_begin + 1;
}

create_fiber_func_t* g_create_fiber_orig = nullptr;
LPVOID WINAPI create_fiber_hook(SIZE_T stack_size, LPFIBER_START_ROUTINE func, LPVOID arg)
{
    LPVOID fiber = g_create_fiber_orig(stack_size, func, arg);
    assert(g_fiber_map_weak.find(fiber) == g_fiber_map_weak.end());
    transfer_ownership(fiber);
    return fiber;
}

delete_fiber_func_t* g_delete_fiber_orig = nullptr;
VOID WINAPI delete_fiber_hook(LPVOID fiber)
{
    g_fiber_map_owner.erase(fiber);
}

}

void init()
{
    assert(g_create_fiber_orig == nullptr);

    auto import = ::GetProcAddress(::GetModuleHandleA("kernel32.dll"), "CreateFiber");
    assert(import);
    g_create_fiber_orig = (create_fiber_func_t*)PatchIAT(
        ::GetModuleHandle(NULL), import, create_fiber_hook
    );
    assert(g_create_fiber_orig);

    import = ::GetProcAddress(::GetModuleHandleA("kernel32.dll"), "DeleteFiber");
    assert(import);
    g_delete_fiber_orig = (delete_fiber_func_t*)PatchIAT(
        ::GetModuleHandle(NULL), import, delete_fiber_hook
    );
    assert(g_delete_fiber_orig);
}

void shutdown()
{
    assert(g_create_fiber_orig != nullptr);

    auto res = PatchIAT(
        ::GetModuleHandle(NULL), create_fiber_hook, g_create_fiber_orig
    );
    assert(res != nullptr);
    res = PatchIAT(
        ::GetModuleHandle(NULL), delete_fiber_hook, g_delete_fiber_orig
    );
    assert(res != nullptr);

    g_fiber_map_weak.clear();
    g_fiber_map_owner.clear();

    g_create_fiber_orig = nullptr;
    g_delete_fiber_orig = nullptr;
}

void load_state(LPVOID fiber, fiber_state& state)
{
    assert(g_create_fiber_orig != nullptr);

    load(fiber, state.data);
    state.stack.clear();
    auto found = g_fiber_map_weak.find(fiber);
    assert(found != g_fiber_map_weak.end());
    auto refcount = found->second.lock();
    const auto size = refcount->stack_size;
    state.stack.reserve(size);
    const auto begin = state.data.stack_begin;
    const auto end = state.data.stack_begin + size;
    std::copy(begin, end, std::back_inserter(state.stack));
    std::swap(state.refcount, refcount);
}

void dump_state(const fiber_state& state)
{
    assert(g_create_fiber_orig != nullptr);

    dump_unprotected(state.data, state.refcount->fiber);
    std::copy(state.stack.begin(), state.stack.end(), state.data.stack_begin);
}

namespace
{

struct deleter
{
    void operator()(LPVOID fiber)
    {
        deleter_fiber_func(fiber);
        g_fiber_map_weak.erase(fiber);
    }
    delete_fiber_func_t* deleter_fiber_func;
    
};

}

void transfer_ownership(LPVOID fiber)
{
    assert(g_create_fiber_orig != nullptr);

    if (g_fiber_map_weak.find(fiber) == g_fiber_map_weak.end())
    {
        auto ptr = std::make_shared<fiber_state::refcount_>(
            fiber, deleter{g_delete_fiber_orig}, get_stack_size(fiber)
        );
        g_fiber_map_owner[fiber] = ptr;
        g_fiber_map_weak[fiber] = ptr;
    }
}

}
