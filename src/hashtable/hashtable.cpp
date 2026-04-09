#include "hashtable.h"

#include <assert.h>
#include <stdlib.h>

#include "list.h"

// ================================= CTOR/DTOR ================================

hashtable_ret_e
HashTableCtor(hashtable_t* h_tab,
              size_t       h_size)
{
    assert(h_tab != nullptr);

    *h_tab = (hashtable_t) calloc(1, sizeof(hashtable_s));

    if (*h_tab == nullptr)
    {
        return HT_BAD_ALLOCATION;
    }

    (*h_tab)->tab_size = h_size;

    (*h_tab)->buckets = (size_t*) calloc(h_size, sizeof(size_t));
    if ((*h_tab)->buckets == nullptr)
    {
        free(*h_tab);

        return HT_BAD_ALLOCATION;
    }                                    

    list_return_e list_ctor_output = LIST_RETURN_SUCCESS;
    if ((list_ctor_output = InitList(&(*h_tab)->data, h_size)))
    {
        free((*h_tab)->buckets);
        free(*h_tab);

        return HT_LIST_ERR;
    }

    return HT_SUCCESS;
}

hashtable_ret_e
HashTableDtor(hashtable_t h_tab)
{
    if (h_tab != nullptr)
    {
        DestroyList(h_tab->data);
        free(h_tab->buckets);

        free(h_tab);
    }

    return HT_SUCCESS;
}
                                                                       
// ============================= HASHTABLE_FUNCTION ===========================

// hashtable_ret_e 
// HashTable
