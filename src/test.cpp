#include "hashtable.h"
#include "list.h"

#include <stdio.h>

int main(void)
{

    hashtable_t hash_tab = nullptr;
    HashTableCtor(&hash_tab, 10);


    size_t meow;
    ListInitNewElem(hash_tab->data, 1, &meow);



    HashTableDtor(hash_tab);


    return 0;
}                                           
