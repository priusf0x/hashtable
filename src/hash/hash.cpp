#include "hash.h"

#include <assert.h>
#include <stdint.h>

#include "my_string.h"

// ================================ HASH_ALGOS ================================


// 3. First ASCII hash 

uint64_t 
HashFirstAscii(string_s string)
{
    return (uint64_t) string.string[0];
}

