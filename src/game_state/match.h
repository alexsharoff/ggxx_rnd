#include <cstdint>


struct match
{
    uint8_t p1_rounds_won;
    uint8_t p2_rounds_won;
    uint32_t round_end_bitmask;

    void read(size_t base);
    void write(size_t base) const;
};
