#include "hashtable.h"
#include "list.h"

#include <stdio.h>

int main(void)
{

    hashtable_t ht = nullptr;
    HashTableCtor(&ht, 10);

    // size_t meow;
    // ListInitNewElem(hash_tab->data, 1, &meow);
 
    HashTableDump(ht, "meow");

    HashTableDtor(ht);


    return 0;
}                                           
