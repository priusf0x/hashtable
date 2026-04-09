#ifndef HASHTABLE_H

#include "list.h"

#include <stdlib.h>

struct hashtable_s 
{
    size_t        tab_size;
    size_t*       buckets;
    list_t        data;
    int           (*hash_func) (const char*); 
};
typedef hashtable_s* hashtable_t;

// ================================ RETURN_ENUN ===============================

enum hashtable_ret_e 
{
    HT_SUCCESS,
    HT_BAD_ALLOCATION,
    HT_LIST_ERR,
    HT_FILE_OPEN_ERR,
    HT_FILE_CLOSE_ERR
};

// ================================= CTOR/DTOR ================================

hashtable_ret_e
HashTableCtor(hashtable_t* h_tab, size_t default_size);

hashtable_ret_e
HashTableDtor(hashtable_t h_tab);


#endif // HASHTABLE_H
