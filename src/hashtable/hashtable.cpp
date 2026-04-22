#include "hashtable.h"

#include <assert.h>
#include <scoped_allocator>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <immintrin.h>

#include "buffer/buffer.h"
#include "hash.h"
#include "list.h"
#include "buffer.h"
#include "string.h"
#include "tools.h"

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

inline uint32_t  
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

size_t 
GetIndex(hashtable_t ht, 
         string_s    elem)
{
    assert(ht != nullptr);

    return HashCRC32(elem) % TABLE_SIZE;
}

////////////////////////////// inlining_list_functions ////////////////////////

static inline void 
InlinedGetValue(list_element_s* data,
                size_t       element_index,
                data_type*   value)
{
    *value = data[element_index].element;
}

static inline ssize_t
InlinedGetNextElement(list_element_s* data,
                      size_t element_index)
{

    return data[element_index].next;
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
            next_index = (size_t) GetNextElement(list, list_index);
            GetElementValue(list, list_index, &cmp_string);
            if ((cmp_string.size == elem.size) && 
                !ssestrncmp(cmp_string.string, elem.string, elem.size))
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
    list_element_s* data = ht->data->data;
    string_s cmp_string = {};
                 
    if (list_index == 0) [[unlikely]]
    {
        return HT_NO_SUCH_ELEM; 
    }
    else 
    {
        size_t next_index = list_index;
        size_t elem_size = elem.size;
        const char* elem_ptr = elem.string;
        do
        {
            list_index = next_index;
            next_index = (size_t) InlinedGetNextElement(data, list_index);
            InlinedGetValue(data, list_index, &cmp_string);
            if ((cmp_string.size == elem_size)
                    && !strncmp(cmp_string.string, elem_ptr, elem_size))
            {
                return HT_SUCCESS;
            }   
        } while (next_index != 0);
    }

    return HT_NO_SUCH_ELEM;
}

// ========================= LINEARIZE_HASHTABLE ============================

static list_return_e
LinearizeList(list_t  old_list,
              list_t  new_list,
              size_t* first_ind)
{
    assert(old_list != nullptr);
    assert(new_list != nullptr);
    assert(first_ind != nullptr);

    if (*first_ind == 0)
    {
        return LIST_RETURN_SUCCESS;
    }

    size_t old_index = *first_ind;
    size_t new_index = 0;
    string_s value = {};
    list_return_e output = LIST_RETURN_SUCCESS;

    GetElementValue(old_list, old_index, &value);
    if ((output = ListInitNewElem(new_list, value, &new_index)))
    {
        return output;
    }
    *first_ind = new_index;
    old_index = (size_t) GetNextElement(old_list, old_index);

    while (old_index)
    {
        GetElementValue(old_list, old_index, &value);
        if ((output = ListAddAfterElement(new_list, value, new_index)))
        {
            return output;
        }
        old_index = (size_t) GetNextElement(old_list, old_index);
        new_index = (size_t) GetNextElement(new_list, new_index);
    }

    return LIST_RETURN_SUCCESS;
}

hashtable_ret_e 
HashTableLinearize(hashtable_t ht)
{
    assert(ht != nullptr);

    list_t lin_data = nullptr;
    list_t old_list = ht->data;
    InitList(&lin_data, old_list->elements_capacity);

    size_t buck_amount = ht->tab_size;
    for (size_t buck = 0; buck < buck_amount; buck++)
    {
        if (LinearizeList(old_list, lin_data, &ht->buckets[buck]))
        {
            return HT_LIST_ERR;
        }
    }

    DestroyList(old_list);
    ht->data = lin_data;

    return HT_SUCCESS;
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
