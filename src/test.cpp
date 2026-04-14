#include "hashtable.h"

#include <cstddef>
#include <stdio.h>

#include "hash.h"
#include "buffer.h"

int main(void)
{

    hashtable_t ht = nullptr;
    HashTableCtor(&ht, TABLE_SIZE, HashCRC32);

    buffer_t src_buf = nullptr;
    BufferCtor(&src_buf, "assets/BrayantOHallaron.txt");
    HashTableLoadFromFile(ht, src_buf);
    buffer_t test_buf = nullptr;
    const size_t test_amount = 1'000'000;
    string_s* tests = (string_s*) calloc(test_amount, sizeof(string_s));
    BufferCtor(&test_buf, "tests/test.txt");
    const size_t test_series = 100;
    HashTableInitTests(tests , test_buf, test_amount);
    HashTableRunTests(ht, tests, test_amount, test_series, "data.json");

    BufferDtor(test_buf);
    free(tests);

    BufferDtor(src_buf);
    HashTableDtor(ht);

    return 0;
}

