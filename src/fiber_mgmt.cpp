#include "fiber_mgmt.h"

#include "patch_iat.h"
#include "memory_dump.h"

#include <cassert>
#include <limits>
#include <stdexcept>


namespace fiber_mgmt
{

namespace
{

fiber_service* g_service = nullptr;

size_t get_stack_size(LPVOID fiber)
{
    assert(fiber != nullptr);
    auto state = reinterpret_cast<const fiber_state::mutable_state*>(fiber);
    size_t* end = state->esp;
    if ((size_t)state->seh_record != std::numeric_limits<size_t>::max())
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
        end = state->seh_record;
    }
    return end - state->stack_begin + 1;
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
    memory_dump::load(fiber, state.data);
    state.stack.clear();
    auto found = m_weak_map.find(fiber);
    assert(found != m_weak_map.end());
    state.shared = found->second.lock();
    const auto size = state.shared->stack_size;
    state.stack.reserve(size);
    const auto begin = state.data.stack_begin;
    const auto end = state.data.stack_begin + size;
    std::copy(begin, end, std::back_inserter(state.stack));
}

void fiber_service::restore(const fiber_state& state) const
{
    memory_dump::dump_unprotected(state.data, state.shared->fiber);
    std::copy(state.stack.begin(), state.stack.end(), state.data.stack_begin);
}

LPVOID fiber_service::create_fiber(SIZE_T stack_size, LPFIBER_START_ROUTINE func, LPVOID arg)
{
    LPVOID fiber = m_create_fiber_func(stack_size, func, arg);
    if (fiber == NULL)
    {
        auto error_id = ::GetLastError();
        char message[1024] = {0};
        auto res = ::FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, error_id, 0, (LPSTR)&message, sizeof(message), NULL
        );
        (void)res;
        assert(res != 0);
        throw std::runtime_error(message);
    }
    transfer_ownership(fiber);
    return fiber;
}

void fiber_service::transfer_ownership(LPVOID fiber)
{
    auto found = m_weak_map.find(fiber);
    assert(found == m_weak_map.end());
    auto ptr = std::make_shared<const immutable_fiber_state>(
        fiber, get_stack_size(fiber), shared_from_this()
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
    LPVOID fiber_, size_t stack_size_, fiber_service::ptr_t service_
) : fiber(fiber_), stack_size(stack_size_), service(service_)
{
    assert(fiber != nullptr);
    assert(stack_size != 0);
    assert(service != nullptr);
}

immutable_fiber_state::~immutable_fiber_state()
{
    if (service)
        service->destroy(fiber);
}

}
