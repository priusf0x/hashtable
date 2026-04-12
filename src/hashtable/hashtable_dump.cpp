#include "hashtable.h"

#include <assert.h>
#include <cstddef>
#include <functional>
#include <istream>
#include <stdlib.h>
#include <stdio.h>

#include "list.h"
#include "tools.h"

// =============================== LOGGER =====================================

// static hashtable_ret_e PrintHTMLHeader(const char* current_time);

/////////////////////////////// singletone ////////////////////////////////////
static const char* LOG_FILE_NAME = "logs/log_file.htm";
static FILE*
HashTableGetLogFile()
{
    static FILE* log_file = fopen(LOG_FILE_NAME, "w+");
    return log_file;
}

static hashtable_ret_e                                    
HashTableDot(const hashtable_t ht,
             const char*       current_time);

hashtable_ret_e
HashTableDump(const hashtable_t ht,
              const char*       comment)
{
    assert(ht != nullptr);
    assert(comment != nullptr);

    FILE* log_file = HashTableGetLogFile();
    if (log_file == nullptr)
    {
        return HT_FILE_OPEN_ERR;
    }

    const size_t str_time_size = 100;
    char current_time[str_time_size] = {};
    GetTime(current_time, str_time_size);

    // PrintHTMLHeader(current_time);
    fprintf(log_file, "<h4>Comment:\"%s\"</h4>", comment);
    HashTableDot(ht, current_time);

    return HT_SUCCESS;
}
//////////////////////////////////// dot_gen //////////////////////////////////

// static void DrawFilledElement(const list_t list, size_t  index, FILE* dot_file);
// static void DrawEmptyElement(const list_t list, size_t  index, FILE* dot_file);
// static void DrawInfoElements(const list_t list, FILE* dot_file);

#pragma GCC diagnostic ignored "-Wformat-nonliteral"

static hashtable_ret_e 
HashTableDotInit(FILE* dot_file);
static hashtable_ret_e
HashTableDotEnd(FILE* dot_file);
static hashtable_ret_e
HashTableDrawBuckets(const hashtable_t ht,
                     FILE*             dot_file);
static hashtable_ret_e
HashTableCompileDot(const char* current_time);

static hashtable_ret_e                                    
HashTableDot(const hashtable_t ht,
             const char*       current_time)
{
    assert(ht != nullptr);
    assert(current_time != nullptr);

    const ssize_t max_string_size = 40;
    char file_name[max_string_size] = {};
    const char* name_template = "logs/%s.gv";

    snprintf(file_name, max_string_size - 1, name_template, current_time);
    FILE* dot_file = fopen(file_name, "w+");
    if (dot_file == nullptr)
    {
        return HT_FILE_OPEN_ERR;
    }

    HashTableDotInit(dot_file);
    HashTableDrawBuckets(ht, dot_file);
    HashTableDotEnd(dot_file);

    if (fclose(dot_file) != 0)
    {
        return HT_FILE_CLOSE_ERR;
    }

    HashTableCompileDot(current_time);

    return HT_SUCCESS;
}

static hashtable_ret_e
HashTableDotInit(FILE* dot_file)
{
    assert(dot_file != nullptr);

    const char* graph_init = "graph  G{ bgcolor = \"#303030\";"
                             "splines = ortho; node [pin = \"true\","
                             "shape = box,"
                             "style = filled,]";
    fprintf(dot_file, "%s", graph_init);

    return HT_SUCCESS;
}

static hashtable_ret_e
HashTableDotEnd(FILE* dot_file)
{
    assert(dot_file != nullptr);

    const char* end_sentence = "}";
    fprintf(dot_file, "%s", end_sentence);

    return HT_SUCCESS;
}

static hashtable_ret_e
HashTableCompileDot(const char* current_time)
{
    assert(current_time != nullptr);

    const ssize_t max_command_size = 256;
    char command[max_command_size] = {};
    snprintf(command, max_command_size - 1, "neato -Tpng logs/%s.gv -o"
                        "logs/%s.png", current_time, current_time);
    system(command);

    return HT_SUCCESS;
}

static const double STEP_Y = 0.52;
static const double STEP_X = 2.02;

static hashtable_ret_e
DrawData(const hashtable_t ht, 
         size_t            buc_num,
         FILE*             dot_file)
{
    assert(ht != nullptr);
    assert(dot_file != nullptr);
    
    list_t list = ht->data;
    const char* data_template = "d%zu%zu[ fillcolor = \"#949494\","
                                "label = \"word = %.*s\", width = 2"
                                ",pos = \"%f, %f\"];\n";
    ssize_t index = (ssize_t) ht->buckets[buc_num]; 
    string_s word = {};
    size_t count = 0;

    while (index > 0)
    {
        count++;

        const double pos_y = (double) (ht->tab_size - buc_num) * STEP_Y;
        double pos_x = (double) count * STEP_X;

        GetElementValue(ht->data, index, &word);
        fprintf(dot_file, data_template, buc_num, index, 
                    word.size, word.string, pos_x, pos_y);

        index = GetNextElement(list, (size_t) index);
    }


    return HT_SUCCESS;
}

static hashtable_ret_e
DrawBucket(const hashtable_t ht, 
           size_t            buc_num,
           FILE*             dot_file)
{
    assert(ht != nullptr);
    
    const double pos_y = (double) (ht->tab_size - buc_num) * STEP_Y;
    const double pos_x = 0;

    const char* bucket_template = "b%zu[ fillcolor = \"#b16261\","
                                  "label = \"Bucket %ld\", width = 1.8"
                                  ",pos = \"%f, %f!\"];\n";

    fprintf(dot_file, bucket_template, buc_num, buc_num, pos_x, pos_y);

    DrawData(ht, buc_num, dot_file);


    return HT_SUCCESS; 
}

static hashtable_ret_e
HashTableDrawBuckets(const hashtable_t ht,
                     FILE*             dot_file)
{
    assert(ht != nullptr);
    assert(dot_file != nullptr);

    for (size_t buc_num = 0; buc_num < ht->tab_size; buc_num++)
    {
        DrawBucket(ht, buc_num, dot_file);
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
HashTableWriteInJSON(hashtable_t ht,
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
// Коля, если ты это читаешь прости меня, пожалуйста, мне очень стыдно
// здесь использовать каллок, но я не хотел делать VLA или массивы  константной длины

    for (size_t buc_num = 0; buc_num < tab_size; buc_num++)
    {
        dist_data[buc_num] = CountElInBucket(ht, buc_num);
    }

    HashTableWriteInJSON(ht, dist_data,output_name);

    free(dist_data);

    return HT_SUCCESS;
}

// ================================ JSON WRITER ===============================

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
HashTableWriteInJSON(hashtable_t ht,
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

    const size_t tab_size = ht->tab_size;
    for (size_t buc_num = 0; buc_num < tab_size - 1; buc_num++)
    {
        fprintf(output_file, "%zu,\n", dist_data[buc_num]);
    }
    fprintf(output_file, "%zu", dist_data[tab_size - 1]);

    PrintJSONEOF(output_file);

    if (fclose(output_file))
    {
        return HT_FILE_CLOSE_ERR;
    }

    return HT_SUCCESS;
}


#pragma GCC diagnostic warning "-Wformat-nonliteral"
