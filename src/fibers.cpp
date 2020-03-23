
#include "fibers.h"

#include "patch_iat.h"
#include "memory_dump.h"

#include <string>
#include <unordered_map>


namespace fibers
{

namespace
{

using memory_dump::load;
using memory_dump::dump;

typedef LPVOID (WINAPI create_fiber_func_t)(SIZE_T, LPFIBER_START_ROUTINE, LPVOID);
typedef VOID (WINAPI delete_fiber_func_t)(LPVOID);

std::unordered_map<LPVOID, size_t> g_fiber_map;

create_fiber_func_t* g_create_fiber_orig = nullptr;
LPVOID WINAPI create_fiber_hook(SIZE_T stack_size, LPFIBER_START_ROUTINE func, LPVOID arg)
{
    LPVOID fiber = g_create_fiber_orig(stack_size, func, arg);
    assert(g_fiber_map.find(fiber) == g_fiber_map.end());
    fiber_state::data_ data;
    load(fiber, data);
    g_fiber_map[fiber] = data.stack_end - data.stack_begin;
    return fiber;
}

delete_fiber_func_t* g_delete_fiber_orig = nullptr;
VOID WINAPI delete_fiber_hook(LPVOID fiber)
{
    // postpone deletion
}

const char* get_current_module_name()
{
    static char s_filename[64] = {0};
    if (!s_filename)
        ::GetModuleFileNameA(::GetModuleHandle(nullptr), s_filename, sizeof(s_filename));
    return s_filename;
}

}

void init()
{
    const auto module_name = get_current_module_name();
    g_create_fiber_orig = PatchIAT(
        "kernel32.dll", "CreateFiber", module_name, create_fiber_hook
    );
    g_delete_fiber_orig = PatchIAT(
        "kernel32.dll", "DeleteFiber", module_name, delete_fiber_hook
    );
}

void shutdown()
{
    const auto module_name = get_current_module_name();
    PatchIAT(
        "kernel32.dll", "CreateFiber", module_name, g_create_fiber_orig
    );
    PatchIAT(
        "kernel32.dll", "DeleteFiber", module_name, g_delete_fiber_orig
    );

    for (const auto [fiber, _] : g_fiber_map)
        g_delete_fiber_orig(fiber);
    g_fiber_map.clear();
}

void load_state(LPVOID fiber, fiber_state& state)
{
    load(fiber, state.data);
    state.stack.clear();
    const auto size = g_fiber_map[fiber];
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

void free(LPVOID fiber)
{
    g_delete_fiber_orig(fiber);
    g_fiber_map.erase(fiber);
}

}
