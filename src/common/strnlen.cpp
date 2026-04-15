#include "tools.h"
#include <emmintrin.h>
#include <nmmintrin.h>
#include <assert.h>
#include <smmintrin.h>  
#include <stdlib.h>

// int
// ssestrncmp(const char* s1,
//            const char* s2,
//            int         n)
// {
//     assert(s1 != nullptr);
//     assert(s2 != nullptr);
//
//     int is_not_equal = 0; 
//
//     while ((n > 0) && !is_not_equal)
//     {
//         __m128 s1_m = _mm_loadu_si128((const __m128i_u*) s1);
//         __m128 s2_m = _mm_loadu_si128((const __m128i_u*) s2);
//         is_not_equal = _mm_cmpestrc(s1_m, n, s2_m, n, _SIDD_CMP_EQUAL_EACH |_SIDD_NEGATIVE_POLARITY);
//         n -= 16;
//         s1 += 16;
//         s2 += 16;
//     }
//
//     return is_not_equal;
// }

int
ssestrncmp(const char* s1,
           const char* s2,
           int         n)
{
    assert(s1 != nullptr);
    assert(s2 != nullptr);

    if (n <= 16) [[likely]]
    {
        __m128 s1_m = _mm_loadu_si128((const __m128i_u*) s1);
        __m128 s2_m = _mm_loadu_si128((const __m128i_u*) s2);
        return _mm_cmpestrc(s1_m, n, s2_m, n, _SIDD_CMP_EQUAL_EACH |_SIDD_NEGATIVE_POLARITY);
    }

    int is_not_equal = 0;
    while ((n > 0) && !is_not_equal)
    {
        __m128 s1_m = _mm_loadu_si128((const __m128i_u*) s1);
        __m128 s2_m = _mm_loadu_si128((const __m128i_u*) s2);
        is_not_equal = _mm_cmpestrc(s1_m, n, s2_m, n, _SIDD_CMP_EQUAL_EACH |_SIDD_NEGATIVE_POLARITY);
        n -= 16;
        s1 += 16;
        s2 += 16;
    }

    return is_not_equal;
}
