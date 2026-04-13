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

#pragma GCC diagnostic warning "-Wformat-nonliteral"
