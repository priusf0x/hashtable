#include "hashtable.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "tools.h"

// =============================== LOGGER =====================================

static hashtable_ret_e PrintHTMLHeader(const char* current_time);
static hashtable_ret_e HashTableDot(const hashtable_t ht, const char* current_time);

/////////////////////////////// singletone ////////////////////////////////////
static const char* LOG_FILE_NAME = "logs/log_file.htm";
static FILE*
HashTableGetLogFile()
{
    static FILE* log_file = fopen(LOG_FILE_NAME, "w+");
    return log_file;
}

hashtable_ret_e
HashTableDump(const hashtable_t ht,
              const char*       comment)
{
    assert(ht != nullptr);
    assert(comment != nullptr);

    FILE* log_file = GetLogFile();
    if (log_file == nullptr)
    {
        return HT_FILE_OPEN_ERR;
    }

    const size_t str_time_size = 100;
    char current_time[str_time_size] = {};
    GetTime(current_time, str_time_size);

    PrintHTMLHeader(current_time);
    fprintf(log_file, "<h4>Comment:\"%s\"</h4>", comment);
    HashTableDot(ht, current_time);

    return HT_SUCCESS;
}

//================================ DOT_GEN ====================================

// static void DrawFilledElement(const list_t list, size_t  index, FILE* dot_file);
// static void DrawEmptyElement(const list_t list, size_t  index, FILE* dot_file);
// static void DrawInfoElements(const list_t list, FILE* dot_file);

#pragma GCC diagnostic ignored "-Wformat-nonliteral"

static hashtable_ret_e 
HashTableDotInit(FILE* dot_file);



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
    FILE* dot_file = fopen(name_template, "w+");
    if (dot_file == nullptr)


    {
        return HT_FILE_OPEN_ERR;
    }

    HashTableDotInit(dot_file);

    // HashTableDrawBuckets
    // HashTableDrawData

    if (fclose(dot_file) != 0)
    {
        return HT_FILE_CLOSE_ERR;
    }

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


// // ======================= PRINT_INFO_FUNCTION ===================
//
// static void
// PrintHTMLHeader(FILE*       log_file,
//                 const char* current_time)
// {
//     assert(log_file != nullptr);
//     assert(current_time != nullptr);
//
//     fprintf(log_file, "<html>\n"
//                         "<style>"
//                         "body{background-color: rgb(48, 48, 48);}"
//                         "h1{color: rgb(212, 58, 56);}"
//                         "h2{color: rgba(153, 26, 24, 1);}"
//                         "h4{color: rgb(182, 182, 182);}"
//                         "</style>"
//                         "<h1> LIST_DUMP %s</h1>\n",  current_time);
//
//     const ssize_t max_string_size = 50;
//     char img_template[max_string_size] = {};
//     snprintf(img_template, max_string_size - 1, "<img src=\"%s.png\","
//                                                 "height = \"20%%\">",
//                                                 current_time);
//     fprintf(log_file, "%s", img_template);
// }
//
//
// static void
// DrawFilledElement(const list_t list,
//                   size_t       index,
//                   FILE*        dot_file)
// {
//
//     assert(list != nullptr);
//     assert(dot_file != nullptr);
//
//     fprintf(dot_file, "p%zu[ fillcolor = \"#949494\","
//                         "label = \"prev = %ld\", width = 1.8"
//                         ",pos = \"%zu.05, 10!\"];\n", index,
//                         list->data[index].previous, 4 + 5 * index);
//
//     fprintf(dot_file, "i%zu[fillcolor =\"#b6b6b6ff\","
//                         "label = \"index = %zu\", width = 3.7,"
//                         "pos = \"%zu, 11.2!\"];", index,
//                         index, 5 + 5 * index);
//
//     fprintf(dot_file, "v%zu[fillcolor =\"#b16261\","
//                         "label = \"value = %f\"width = 3.7, "
//                         "pos = \"%zu,10.6!\"];", index,
//                         list->data[index].element, 5 + 5 * index);
//
//     fprintf(dot_file, "n%zu[fillcolor =\"#949494\","
//                         "label = \"next = %ld\", width = 1.8,"
//                         " pos = \"%zu.95,10!\"];",
//                         index, list->data[index].next,5 + 5 * index);
//
//     fprintf(dot_file, "inv%zu[style=\"invis\","
//                         "height = 2, pos = \"%zu.5, 11!\"];",
//                         index, 7 + 5 * index);
//
//     if ((list->data[index].next != 0) && (list->data[index].next != NO_LINK))
//     {
//         fprintf(dot_file, "n%zu -- p%ld[color = \"#d1d1d1\", dir = both];",
//                 index, list->data[index].next);
//     }
// }
//
// static void
// DrawEmptyElement(const list_t list,
//                  size_t       index,
//                  FILE*        dot_file)
// {
//     assert(dot_file != nullptr);
//
//     fprintf(dot_file, "p%zu[fillcolor = \"#818181ff\","
//                       "label = \"prev = %ld\", width = 1.8"
//                       ",pos = \"%zu.05, 10!\"];", index,
//                       list->data[index].previous, 4 + 5 * index);
//
//     fprintf(dot_file, "i%zu[fillcolor =\"#818181ff\","
//                       "label = \"index = %zu\","
//                       "width = 3.7,pos = \"%zu, 11.2!\"];", index,
//                       index, 5 + 5 * index);
//
//     fprintf(dot_file, "v%zu[fillcolor =\"#818181ff\","
//                       "label = \"value = %f\",width = 3.7,"
//                       "pos = \"%zu,10.6!\"];", index,
//                       list->data[index].element, 5 + 5 * index);
//
//     fprintf(dot_file, "n%zu[fillcolor =\"#818181ff\","
//                       "label = \"next = %ld\", width = 1.8,"
//                       " pos = \"%zu.95,10!\"];", index,
//                       list->data[index].next,5 + 5 * index);
//
//     fprintf(dot_file, "inv%zu[style=\"invis\","
//                       "height = 2, pos = \"%zu.5, 11!\"];",
//                       index, 7 + 5 * index);
//
//     if (list->data[index].next != NO_LINK)
//     {
//         fprintf(dot_file, "n%zu -- p%ld[color = \"#aaaaaa96\","
//                 "dir = forward];", index, list->data[index].next);
//     }
// }

#pragma GCC diagnostic warning "-Wformat-nonliteral"
