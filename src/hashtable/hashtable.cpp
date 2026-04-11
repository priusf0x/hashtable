#include "hashtable.h"

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "buffer/buffer.h"
#include "list.h"
#include "buffer.h"
#include "string.h"

// ================================= CTOR/DTOR ================================

hashtable_ret_e
HashTableCtor(hashtable_t* h_tab,
              size_t       h_size,
              uint64_t   (*hash_func) (string_s))
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
    if ((list_ctor_output = InitList(&(*h_tab)->data, h_size * 3)))
    {
        free((*h_tab)->buckets);
        free(*h_tab);

        return HT_LIST_ERR;
    }

    (*h_tab)->hash_func = hash_func;

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

static inline size_t 
GetIndex(hashtable_t ht, 
         string_s    elem)
{
    assert(ht != nullptr);

    return ht->hash_func(elem) % ht->tab_size;
}

static inline size_t
MaxStringSize(string_s s1, 
              string_s s2)
{
    return s1.size > s2.size ? s1.size : s2.size;
}



hashtable_ret_e 
HashTableAddElem(hashtable_t ht,
                 string_s    elem)
{
    assert(ht != nullptr);

    size_t table_index = GetIndex(ht, elem);
    size_t list_index = ht->buckets[table_index];
    list_t list = ht->data;
    string_s cmp_string = {};
    bool is_in_table = false;
    
    if (list_index == 0)
    {
        ListInitNewElem(list, elem, &list_index);
        ht->buckets[table_index] = list_index;
    }
    else 
    {
        size_t next_index = list_index;

        do
        {
            list_index = next_index;
            next_index = (size_t) GetNextElement(list, list_index);
            GetElementValue(list, list_index, &cmp_string);
            if (!strncmp(cmp_string.string, elem.string, 
                            MaxStringSize(cmp_string, elem)))
            {
                is_in_table = true;
                break;
            }   
        } while (next_index != 0);

        if (!is_in_table)
        {
            ListAddAfterElement(list, elem, list_index);
        }
    }

    return HT_SUCCESS;
}

hashtable_ret_e 
HashTableGetElem(hashtable_t ht,
                 string_s    elem)
{
    assert(ht != nullptr);

    size_t table_index = GetIndex(ht, elem);
    size_t list_index = ht->buckets[table_index];
    list_t list = ht->data;
    string_s cmp_string = {};
    
    if (list_index == 0)
    {
        return HT_NO_SUCH_ELEM; 
    }
    else 
    {
        size_t next_index = list_index;
        do
        {
            list_index = next_index;
            next_index = (size_t) GetNextElement(list, list_index);
            GetElementValue(list, list_index, &cmp_string);
            if (!strncmp(cmp_string.string, elem.string, 
                            MaxStringSize(cmp_string, elem)))
            {
                return HT_SUCCESS;
            }   
        } while (next_index != 0);
    }

    return HT_NO_SUCH_ELEM;
}

// ======================== LOAD_TABLE_FROM_FILE ==============================

hashtable_ret_e
HashTableLoadFromFile(hashtable_t ht, 
                      buffer_t    buf)
{
    assert(ht != nullptr);
    assert(buf != nullptr);

    hashtable_ret_e ht_ret = HT_SUCCESS;
    SkipNotAlphaB(buf);

    while  (*(buf->buffer + buf->cur_pos))
    {
        size_t word_pos = buf->cur_pos;
        SkipAlphaB(buf);
        size_t word_size = buf->cur_pos - word_pos;
        SkipNotAlphaB(buf);

        string_s word = {buf->buffer + word_pos, word_size};
        if ((ht_ret = HashTableAddElem(ht, word)))
        {
            return ht_ret;
        }
    }

    return HT_SUCCESS;
}
