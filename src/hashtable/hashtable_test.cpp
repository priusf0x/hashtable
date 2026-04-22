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

static hashtable_ret_e
PrintJSONHeader(FILE* output_file) 
{
    assert(output_file != nullptr);

    const char* json_header = 
        "{\n"
        "\"data\": [\n";
        
    fprintf(output_file, "%s", json_header);

    return HT_SUCCESS;
}

static hashtable_ret_e
PrintJSONEOF(FILE* output_file)
{
    assert(output_file != nullptr);

    const char* eof_text = "]}";
    fprintf(output_file, "%s", eof_text);

    return HT_SUCCESS;
}

static hashtable_ret_e 
PrintJSONData(FILE*   output_file,
              size_t* data,
              size_t  test_series)
{
    assert(output_file != nullptr);
    assert(data != nullptr);

    for (size_t i = 0; i < test_series - 1; i++)
    {
        fprintf(output_file, "%zu,\n", data[i]);
    }
    fprintf(output_file, "%zu\n", data[test_series-1]);


    return HT_SUCCESS;
}

///////////////////////////////// run_tests ///////////////////////////////////

inline uint64_t rdtsc() {
	uint64_t lo = 0, hi = 0;
	asm volatile ( "rdtsc\n" : "=a" (lo), "=d" (hi) );
	return ((uint64_t)hi << 32) | lo;
}

hashtable_ret_e 
HashTableRunTests(hashtable_t ht,
                  string_s*   ht_test,
                  size_t      word_amount,
                  size_t      test_series,
                  const char* output_name)
{
    assert(ht != nullptr);
    assert(ht_test != nullptr);
    assert(output_name != nullptr);

    FILE* output_file = fopen(output_name, "w+");
    if (output_file == nullptr)
    {
        return HT_FILE_OPEN_ERR;
    }

    size_t* data = (size_t*) calloc(test_series, sizeof(double));

    if (data == nullptr)
    {
        fclose(output_file);

        return HT_BAD_ALLOCATION;
    }

    size_t undef_amount = 0;

    for (size_t ser_n = 0; ser_n < test_series; ser_n++)
    {
        size_t start = rdtsc();
        for (size_t i = 0; i < word_amount; i++)
        {
            HashTableGetElem(ht, ht_test[i]) ? undef_amount++ : 0;
        }
        size_t stop = rdtsc();
        data[ser_n] = stop - start;
    }

// Print percentage of undefined to make 
    double undef_rate = 100.f * (double) undef_amount 
                            / (double) (test_series * word_amount); 
    fprintf(stdout, "%0.2f\n", undef_rate);

    PrintJSONHeader(output_file);
    PrintJSONData(output_file, data, test_series);
    PrintJSONEOF(output_file);

    free(data);

    if (fclose(output_file))
    {
        return HT_FILE_CLOSE_ERR;
    }

    return HT_SUCCESS;
}

// =========================== DATA_FOR_DISTRIBUTION ==========================

static size_t 
CountElInBucket(hashtable_t ht, 
                size_t      buc_num)
{
    assert(ht != nullptr);

    size_t el_num = ht->buckets[buc_num];

    if (el_num == 0)
    {
        return 0;
    }

    size_t el_amount = 0;
    do
    {
        el_amount++;
    } while ((el_num = (size_t) GetNextElement(ht->data, el_num)));

    return el_amount;
}

static hashtable_ret_e 
HashTableWriteDistr(hashtable_t ht,
                    size_t*     dist_data,
                    const char* output_name);

hashtable_ret_e 
HashTableMakeDistr(hashtable_t ht,
                   const char* output_name)
{
    assert(ht != nullptr);
    assert(output_name != nullptr);

    const size_t tab_size = ht->tab_size;
    size_t* dist_data = (size_t*) calloc(tab_size, sizeof(size_t));
    if (dist_data == nullptr)
    {
        return HT_BAD_ALLOCATION;    
    }

    for (size_t buc_num = 0; buc_num < tab_size; buc_num++)
    {
        dist_data[buc_num] = CountElInBucket(ht, buc_num);
    }

    HashTableWriteDistr(ht, dist_data,output_name);

    free(dist_data);

    return HT_SUCCESS;
}

// ================================ JSON WRITER ===============================

static hashtable_ret_e 
HashTableWriteDistr(hashtable_t ht,
                    size_t*     dist_data,
                    const char* output_name)
{
    assert(ht != nullptr);
    assert(dist_data != nullptr);
    assert(output_name != nullptr);

    FILE* output_file = fopen(output_name, "w+");
    if (output_file == nullptr)
    {
        return HT_FILE_OPEN_ERR;
    }

    PrintJSONHeader(output_file);
    PrintJSONData(output_file, dist_data, ht->tab_size);
    PrintJSONEOF(output_file);

    if (fclose(output_file))
    {
        return HT_FILE_CLOSE_ERR;
    }

    return HT_SUCCESS;
}



