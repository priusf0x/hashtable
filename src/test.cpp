#include "hashtable.h"

#include <stdio.h>

#include "hash.h"

int main(void)
{

    hashtable_t ht = nullptr;
    HashTableCtor(&ht, 10, HashFirstAscii);

    HashTableAddElem(ht, {"meow", 4}); 
    HashTableAddElem(ht, {"m2", 1}); 
    //
    // HashTableAddElem(ht, "aeow"); 
    // HashTableAddElem(ht, "aeow"); 
    // HashTableAddElem(ht, "aeow"); 
    // HashTableAddElem(ht, "aeow"); 
    //
    // HashTableAddElem(ht, "zeow"); 
    //
    HashTableDump(ht, "meow");

    HashTableDtor(ht);

    return 0;
}

