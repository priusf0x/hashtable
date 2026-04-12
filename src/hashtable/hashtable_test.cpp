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

static hashtable_ret_e 
WriteJSON(FILE*  output_file,
          float* data,
          size_t test_series)
{
    assert(output_file != nullptr);
    assert(data != nullptr);

    const char* json_heading = "{\n\"data\":[";
    const char* json_EOF = "]}";

    fprintf(file_output, "%s", json_heading);
    const char* json_heading = "{\n\"data\":[";


    return HT_SUCCESS;
}

hashtable_ret_e 
HashTableRunTests(hashtable_t ht,
                  string_s*   ht_test,
                  size_t      test_amount,
                  size_t      test_series,
                  const char* output_name)
{
    assert(ht != nullptr);
    assert(ht_test != nullptr);
    assert(output_name != nullptr);

    FILE* file_output = fopen(output_name, "w+");
    if (file_output == nullptr)
    {
        return HT_FILE_OPEN_ERR;
    }

    double* data = (double*) calloc(test_series, sizeof(double));

    if (data == nullptr)
    {
        fclose(file_output);

        return HT_BAD_ALLOCATION;
    }

    while (test_series)
    {
        for (size_t i = 0; i < test_amount; i++)
        {
            HashTableGetElem(ht, ht_test[i]);
        }
    }

    free(data);

    if (fclose(file_output))
    {
        return HT_FILE_CLOSE_ERR;
    }

    return HT_SUCCESS;
}

