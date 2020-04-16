#pragma once

#include <Windows.h>

#include <memory>
#include <unordered_map>
#include <vector>


// 64-bit is not supported
static_assert(sizeof(size_t) == 4);

namespace fiber_mgmt
{

struct fiber_state;
struct const_fiber_state;

class fiber_service : public std::enable_shared_from_this<fiber_service>
{
private:
    struct ctor_key
    {
        explicit ctor_key(int) {}
    };
public:
    using ptr_t = std::shared_ptr<fiber_service>;

    static ptr_t start();

    void load(LPVOID fiber, fiber_state& state) const;
    void restore(const fiber_state& state) const;
    LPVOID create_fiber(SIZE_T stack_size, LPFIBER_START_ROUTINE func, LPVOID arg);
    void transfer_ownership(LPVOID fiber);
    void release(LPVOID fiber);

    explicit fiber_service(const ctor_key&);
    ~fiber_service();
    fiber_service(const fiber_service&) = delete;
    fiber_service& operator=(const fiber_service&) = delete;

private:
    void destroy(LPVOID fiber);

    using owner_map_t = std::unordered_map<LPVOID, std::shared_ptr<const const_fiber_state>>;
    using weak_map_t = std::unordered_map<LPVOID, std::weak_ptr<const const_fiber_state>>;

    owner_map_t m_owner_map;
    weak_map_t m_weak_map;

    // https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-createfiber
    typedef LPVOID (WINAPI create_fiber_func_t)(SIZE_T, LPFIBER_START_ROUTINE, LPVOID);
    // https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-deletefiber
    typedef VOID (WINAPI delete_fiber_func_t)(LPVOID);

    create_fiber_func_t* m_create_fiber_func;
    delete_fiber_func_t* m_delete_fiber_func;

    friend const_fiber_state;
};

struct const_fiber_state
{
    const_fiber_state(const const_fiber_state&) = default;
    const_fiber_state(const_fiber_state&&) = default;
    const_fiber_state& operator=(const const_fiber_state&) = default;
    const_fiber_state& operator=(const_fiber_state&&) = default;
    ~const_fiber_state();
    LPVOID fiber;
    size_t stack_size;
    fiber_service::ptr_t service;
};

struct fiber_state
{
    struct mutable_state
    {
        uint8_t pad1[0x4]; // 0
        size_t* seh_record; // 4
        uint8_t pad2[0x4]; // 8
        size_t* stack_begin; // C
        uint8_t pad3[0xC8]; // 10
        size_t* esp; // D8
        uint8_t pad4[0x21C]; // DC
    } data; // 2F8
    std::vector<size_t> stack;
    std::shared_ptr<const const_fiber_state> shared;
};
static_assert(sizeof(fiber_state::mutable_state) == 0x2f8);

}
