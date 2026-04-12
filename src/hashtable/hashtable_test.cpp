#include "hashtable.h"

#include <assert.h>
#include <cstddef>
#include <stdlib.h>

#include "buffer.h"
#include "my_string.h"

// ============================= TEST_FUNCTIONS ===============================

////////////////////////////////// load_tests /////////////////////////////////

hashtable_ret_e 
HashTableInitTests(string_s* ht_test,
                   buffer_t  buf,
                   size_t    test_amount)
{
    assert(ht_test != nullptr);
    assert(buf != nullptr);

    for (size_t i = 0; i < test_amount; i++)
    {
        size_t word_pos = buf->cur_pos;
        SkipAlNumB(buf);
        size_t word_size = buf->cur_pos - word_pos;
        SkipNotAlNumB(buf);

        string_s word = {buf->buffer + word_pos, word_size};
        ht_test[i] = word;
    }

    return HT_SUCCESS;
}
                                      
///////////////////////////////// run_tests ///////////////////////////////////

hashtable_ret_e 
HashTableRunTests(hashtable_t ht,
                  string_s*   ht_test,
                  size_t      test_amount)
{
    assert(ht != nullptr);
    assert(ht_test != nullptr);

    int sum = 0;

    for (size_t i = 0; i < test_amount; i++)
    {
        sum += HashTableGetElem(ht, ht_test[i]);
    }

    printf("%d", sum);

    return HT_SUCCESS;
}

