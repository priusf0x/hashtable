#include "hashtable.h"

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <nmmintrin.h>

#include "buffer/buffer.h"
#include "hash.h"
#include "list.h"
#include "buffer.h"
#include "string.h"

// ================================= CTOR/DTOR ================================

hashtable_ret_e
HashTableCtor(hashtable_t* h_tab,
              size_t       h_size,
              uint32_t   (*hash_func) (string_s))
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

    const size_t load_factor = 10;
    if ((list_ctor_output = InitList(&(*h_tab)->data, h_size * load_factor)))
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

///////////////////////////////// inlining_hash ///////////////////////////////

///////////////////////// intrinsic_hash_implementation ///////////////////////

static inline uint32_t
HashCRCIntrinsics(string_s elem)
{
    uint32_t hash = ~0u;

    for (size_t i = 0; i < elem.size; i++)
    {
        hash = _mm_crc32_u8(hash, (unsigned char) elem.string[i]);
    }

    return hash;
}

inline static size_t 
GetIndex(hashtable_t ht, 
         string_s    elem)
{
    assert(ht != nullptr);

    return HashCRCIntrinsics(elem) % TABLE_SIZE;
}

////////////////////////////// inlining_list_functions ////////////////////////

static inline void 
InlinedGetValue(const list_t list,
                  size_t       element_index,
                  data_type*   value)
{
    *value = list->data[element_index].element;
}

static inline ssize_t
InlinedGetNextElement(const list_t list,
                      size_t       element_index)
{

    return list->data[element_index].next;
}

// for optimization hash function was inlined

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
            next_index = (size_t) InlinedGetNextElement(list, list_index);
            InlinedGetValue(list, list_index, &cmp_string);
            if ((cmp_string.size == elem.size) && 
                !strncmp(cmp_string.string, elem.string, elem.size))
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
            next_index = (size_t) InlinedGetNextElement(list, list_index);
            InlinedGetValue(list, list_index, &cmp_string);
            if ((cmp_string.size == elem.size) && 
                !strncmp(cmp_string.string, elem.string, elem.size))
            {
                return HT_SUCCESS;
            }   
        } while (next_index != 0);
    }

    return HT_NO_SUCH_ELEM;
}

// ======================== LOAD_TABLE_FROM_FILE ==============================

#define HT_GEN_TEST
#ifdef HT_GEN_TEST 
static const char* EXTRACTED_WORDS = "tests/extracted_words.txt";
#endif // HT_GEN_TEST

hashtable_ret_e
HashTableLoadFromFile(hashtable_t ht, 
                      buffer_t    buf)
{
    assert(ht != nullptr);
    assert(buf != nullptr);

    #ifdef HT_GEN_TEST
        FILE* test_file = fopen(EXTRACTED_WORDS, "w+");
    #endif // HT_TEST 

    hashtable_ret_e ht_ret = HT_SUCCESS;
    SkipNotAlNumB(buf);

    while  (*(buf->buffer + buf->cur_pos))
    {
        size_t word_pos = buf->cur_pos;
        SkipAlNumB(buf);
        size_t word_size = buf->cur_pos - word_pos;
        SkipNotAlNumB(buf);

        string_s word = {buf->buffer + word_pos, word_size};
        #ifdef HT_GEN_TEST
        if (HashTableGetElem(ht, word))
        {
            fprintf(test_file, "%.*s\n", (int) word.size, word.string);
        }
        #endif // HT_TEST 
        if ((ht_ret = HashTableAddElem(ht, word)))
        {
            return ht_ret;
        }
    }

    #ifdef HT_GEN_TEST
        fclose(test_file);
    #endif // HT_TEST 

    return HT_SUCCESS;
}
