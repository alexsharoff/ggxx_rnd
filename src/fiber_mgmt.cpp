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
using memory_dump::dump;

typedef LPVOID (WINAPI create_fiber_func_t)(SIZE_T, LPFIBER_START_ROUTINE, LPVOID);
typedef VOID (WINAPI delete_fiber_func_t)(LPVOID);

struct fiber_info
{
    size_t stack_size = 0;
    bool delete_called = false;
};
std::unordered_map<LPVOID, fiber_info> g_fiber_map;

create_fiber_func_t* g_create_fiber_orig = nullptr;
LPVOID WINAPI create_fiber_hook(SIZE_T stack_size, LPFIBER_START_ROUTINE func, LPVOID arg)
{
    LPVOID fiber = g_create_fiber_orig(stack_size, func, arg);
    transfer_ownership(fiber);
    return fiber;
}

delete_fiber_func_t* g_delete_fiber_orig = nullptr;
VOID WINAPI delete_fiber_hook(LPVOID fiber)
{
    const auto found = g_fiber_map.find(fiber);
    if (found == g_fiber_map.end())
    {
        // unknown or unmanaged fiber
        g_delete_fiber_orig(fiber);
    }
    else
    {
        found->second.delete_called = true;
    }
}

}

void init()
{
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
    auto res = PatchIAT(
        ::GetModuleHandle(NULL), create_fiber_hook, g_create_fiber_orig
    );
    assert(res != nullptr);
    res = PatchIAT(
        ::GetModuleHandle(NULL), delete_fiber_hook, g_delete_fiber_orig
    );
    assert(res != nullptr);

    free_all();
}

void load_state(LPVOID fiber, fiber_state& state)
{
    load(fiber, state.data);
    state.stack.clear();
    const auto size = g_fiber_map[fiber].stack_size;
    state.stack.reserve(size);
    const auto begin = state.data.stack_begin;
    const auto end = state.data.stack_begin + size;
    std::copy(begin, end, std::back_inserter(state.stack));
}

void dump_state(LPVOID fiber, const fiber_state& state)
{
    dump(state.data, fiber);
    std::copy(state.stack.begin(), state.stack.end(), state.data.stack_begin);
}

void transfer_ownership(LPVOID fiber)
{
    assert(g_fiber_map.find(fiber) == g_fiber_map.end());
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
    g_fiber_map[fiber].stack_size = end - data.stack_begin + 1;
}

void free(LPVOID fiber)
{
    auto found = g_fiber_map.find(fiber);
    assert(found != g_fiber_map.end());
    if (found->second.delete_called)
        g_delete_fiber_orig(fiber);

    g_fiber_map.erase(found);
}

void free_all()
{
    for (const auto [fiber, _] : g_fiber_map)
        g_delete_fiber_orig(fiber);
    g_fiber_map.clear();
}

}
