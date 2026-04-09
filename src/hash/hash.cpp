#include "hash.h"

#include <assert.h>
#include <stdint.h>

// 3. First ASCII hash 

uint64_t 
HashFirstAscii(const char *string)
{
    assert(string != nullptr);
    
    return (uint64_t) string[0];
}

