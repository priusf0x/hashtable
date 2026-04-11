#include "buffer.h"
#include "hashtable.h"

#include <cstddef>
#include <stdio.h>

#include "hash.h"
#include <ctype.h>

static const size_t TABLE_SIZE = 100;

int main(void)
{

    hashtable_t ht = nullptr;
    HashTableCtor(&ht, TABLE_SIZE, HashFirstAscii);

    buffer_t buf = nullptr;
    BufferCtor(&buf, "assets/BrayantOHallaron.txt");
    HashTableLoadFromFile(ht, buf);

    HashTableMakeDistr(ht);
    HashTableDump(ht, "meoow");

    BufferDtor(buf);
    HashTableDtor(ht);

    return 0;
}
