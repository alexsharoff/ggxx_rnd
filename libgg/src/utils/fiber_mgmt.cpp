#include "fiber_mgmt.h"

#include "iat_patch.h"
#include "memory_dump.h"

#include <cassert>
#include <charconv>
#include <limits>
#include <stdexcept>


namespace fiber_mgmt
{

namespace
{

fiber_service* g_service = nullptr;

void get_stack_region(LPVOID fiber, size_t*& begin, size_t& size)
{
    auto data = reinterpret_cast<fiber_state::mutable_state*>(fiber);
    MEMORY_BASIC_INFORMATION info;
    ::VirtualQuery(data->esp, &info, sizeof(info));
    begin = static_cast<size_t*>(info.BaseAddress);
    size = info.RegionSize / sizeof(size_t);
}

LPVOID WINAPI create_fiber_hook(SIZE_T stack_size, LPFIBER_START_ROUTINE func, LPVOID arg)
{
    assert(g_service != nullptr);
    return g_service->create_fiber(stack_size, func, arg);
}

VOID WINAPI delete_fiber_hook(LPVOID fiber)
{
    assert(g_service != nullptr);
    g_service->release(fiber);
}

}

fiber_service::ptr_t fiber_service::start()
{
    return std::make_shared<fiber_service>(ctor_key{0});
}

fiber_service::fiber_service(const ctor_key&)
{
    assert(!g_service);

    auto import_func = ::GetProcAddress(::GetModuleHandleA("kernel32.dll"), "CreateFiber");
    assert(import_func);
    m_create_fiber_func = (create_fiber_func_t*)PatchIAT(
        ::GetModuleHandle(NULL), import_func, create_fiber_hook
    );
    assert(m_create_fiber_func);

    import_func = ::GetProcAddress(::GetModuleHandleA("kernel32.dll"), "DeleteFiber");
    assert(import_func);
    m_delete_fiber_func = (delete_fiber_func_t*)PatchIAT(
        ::GetModuleHandle(NULL), import_func, delete_fiber_hook
    );
    assert(m_delete_fiber_func);

    g_service = this;
}

fiber_service::~fiber_service()
{
    assert(g_service != nullptr);

    m_weak_map.clear();
    m_owner_map.clear();

    auto res = PatchIAT(
        ::GetModuleHandle(NULL), create_fiber_hook, m_create_fiber_func
    );
    assert(res != nullptr);

    res = PatchIAT(
        ::GetModuleHandle(NULL), delete_fiber_hook, m_delete_fiber_func
    );
    assert(res != nullptr);

    g_service = nullptr;
}

void fiber_service::load(LPVOID fiber, fiber_state& state) const
{
    auto found = m_weak_map.find(fiber);
    assert(found != m_weak_map.end());
    state.shared = found->second.lock();
    memory_dump::load(fiber, state.data);
    size_t stack_size = 0;
    get_stack_region(state.shared->fiber, state.stack_begin, stack_size);
    state.stack.clear();
    state.stack.reserve(stack_size);
    const auto begin = state.stack_begin;
    const auto end = state.stack_begin + stack_size;
    std::copy(begin, end, std::back_inserter(state.stack));
}

void fiber_service::restore(const fiber_state& state) const
{
    size_t* cur_stack_begin = nullptr;
    size_t cur_stack_size = 0;
    get_stack_region(state.shared->fiber, cur_stack_begin, cur_stack_size);
    auto cur_stack_end = cur_stack_begin + cur_stack_size;
    if (cur_stack_begin < state.stack_begin)
    {
        size_t diff = state.stack_begin - cur_stack_begin;
        std::memset(cur_stack_begin, 0, diff);
    }
    std::copy(state.stack.begin(), state.stack.end(), state.stack_begin);
    auto stack_end = state.stack_begin + state.stack.size();
    if (cur_stack_end > stack_end)
    {
        size_t diff = cur_stack_end - stack_end;
        std::memset(stack_end, 0, diff);
    }
    memory_dump::dump_unprotected(state.data, state.shared->fiber);
}

LPVOID fiber_service::create_fiber(SIZE_T stack_size, LPFIBER_START_ROUTINE func, LPVOID arg)
{
    LPVOID fiber = m_create_fiber_func(stack_size, func, arg);
    assert(fiber != NULL);
    if (fiber)
        transfer_ownership(fiber);
    return fiber;
}

void fiber_service::transfer_ownership(LPVOID fiber)
{
    auto found = m_weak_map.find(fiber);
    assert(found == m_weak_map.end());
    auto ptr = std::make_shared<const immutable_fiber_state>(
        fiber, shared_from_this()
    );
    m_owner_map.emplace(fiber, ptr);
    m_weak_map.emplace(fiber, ptr);
}

void fiber_service::release(LPVOID fiber)
{
    m_owner_map.erase(fiber);
}

void fiber_service::destroy(LPVOID fiber)
{
    m_delete_fiber_func(fiber);
    auto found = m_weak_map.find(fiber);
    assert(found != m_weak_map.end());
    m_weak_map.erase(found);
}

immutable_fiber_state::immutable_fiber_state(
    LPVOID fiber_, fiber_service::ptr_t service_
) : fiber(fiber_), service(service_)
{
    assert(fiber != nullptr);
    assert(service != nullptr);
}

immutable_fiber_state::~immutable_fiber_state()
{
    if (service)
        service->destroy(fiber);
}

}
