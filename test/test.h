#include <iostream>

#define TEST(expr) \
    if (!(expr)) { \
        std::cerr << __FILE__ << ':' << __LINE__ << ": " << #expr; \
        return -1; \
    }

#define TEST_EQ(expr1, expr2) { \
    const auto r1 = expr1; \
    const auto r2 = expr2; \
    if (r1 != r2) { \
        std::cerr << __FILE__ << ':' << __LINE__ << ": " \
                  << #expr1 << " <" << r1 << '>' << " != " \
                  << #expr2 << " <" << r2 << '>' << std::endl; \
        return -1; \
    }}
