#include "hash.h"

#include <assert.h>
#include <cstdint>
#include <stdint.h>

#include "my_string.h"

// ================================ HASH_ALGOS ================================

// 1. Always zero

uint32_t
HashAlwaysZero([[maybe_unused]] string_s string)
{
    return 0;
}

// 2. First ASCII hash 

uint32_t 
HashFirstAscii(string_s string)
{
    return (uint32_t) string.string[0];
}

// 3. Word Length hash  

uint32_t 
HashWordLength(string_s string)
{
    return (uint32_t) string.size;
}

// 4. Elem sum

uint32_t 
HashElemSum(string_s string)
{
    uint32_t sum = 0;

    for (size_t i = 0; i < string.size; i++)
    {
        sum += (uint32_t) string.string[i];
    }


    return sum;
}

// 5. Hash Rol

uint32_t 
HashRol(string_s string)
{
    uint32_t hash = 0;
    uint32_t rolled_bit = 0;

    for (size_t i = 0; i < string.size; i++)
    {
        rolled_bit = hash << (sizeof(uint32_t) - 1);
        hash >>= 1;
        hash |= rolled_bit;
        hash ^= (uint32_t) string.string[i];
    }


    return hash;
}

// 6. Hash RoR

uint32_t 
HashRor(string_s string)
{
    uint32_t hash = 0;
    uint32_t rolled_bit = 0;

    for (size_t i = 0; i < string.size; i++)
    {
        rolled_bit = hash >> (sizeof(uint32_t) - 1);
        hash <<= 1;
        hash |= rolled_bit;
        hash ^= (uint32_t) string.string[i];
    }

    return hash;
}

// 7. Hash ELF

uint32_t 
HashGNU(string_s string)
{
    uint32_t hash = 0;

    for (size_t i = 0; i < string.size; i++)
    {
        hash = ((hash << 5) + hash) + (uint32_t) string.string[i];
    }

    return hash;
}

// 8. Hash CRC32

uint32_t 
HashCRC32(string_s string)
{
    uint32_t crc = ~0u;
    const uint32_t filter = 0xEDB88320;

    for (size_t i = 0; i < string.size ; i++) 
    {
        crc ^= (uint32_t) string.string[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) 
            {
                crc = (crc >> 1) ^ filter;
            }
            else 
            {
                crc >>= 1;
            }
        }
    }

    return ~crc;
}

// Просчитанный алгоритм crc32 скопированный с https://web.mit.edu/freebsd/head/sys/libkern/crc32.c


