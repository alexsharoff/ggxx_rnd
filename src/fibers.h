#include "libgg.h"

#include <vector>


namespace fibers
{

struct fiber_state
{
    // SwitchToFiber changes stack_begin, stack_end values.
    // If you need to calculate stack size, do it before
    // SwitchToFiber is called.
    struct data_
    {
        uint8_t data1[0xC]; // 0
        uint32_t* stack_begin; // C
        uint8_t data2[0xC8]; // 10
        uint32_t* stack_end; // D8
        uint8_t data3[0x21C]; // DC
    } data; // 2F8
    // TODO: try to use static array here, stack size should be the same every time
    std::vector<uint32_t> stack;
};
static_assert(sizeof(fiber_state::data_) == 0x2f8);

void init();

void shutdown();

void load_state(LPVOID fiber, fiber_state& state);

void dump_state(LPVOID fiber, const fiber_state& state);

void free(LPVOID fiber);

}
