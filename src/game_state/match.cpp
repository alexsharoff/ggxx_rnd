#include "match.h"
#include "data_mapper.h"


auto g_match_mapper = data_mapper<match>()
    .map(&match::p1_rounds_won, 0x50f7ec)
    .map(&match::p2_rounds_won, 0x50f7ed)
    .map(&match::round_end_bitmask, 0x50f7fc)
;

void match::read(size_t base)
{
    g_match_mapper.read(*this, base);   
}

void match::write(size_t base) const
{
    g_match_mapper.write(*this, base);
}
