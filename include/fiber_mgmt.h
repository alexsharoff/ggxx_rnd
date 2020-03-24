#pragma once

#include <Windows.h>

#include <vector>


namespace fiber_mgmt
{

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
    // TODO: try to use static array here, stack size should be the same every time
    std::vector<size_t> stack;
};
static_assert(sizeof(fiber_state::data_) == 0x2f8);

void init();

void shutdown();

void load_state(LPVOID fiber, fiber_state& state);

void dump_state(LPVOID fiber, const fiber_state& state);

void transfer_ownership(LPVOID fiber);

void free(LPVOID fiber);

void free_all();

}
