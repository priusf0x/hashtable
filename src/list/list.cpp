#include "list.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "tools.h"

const ssize_t CANARY_SIZE = 4;
static const uint64_t CANARY_FILL = 0xBAB1BAB0BAB1BAB0;

static const int NO_LINK = -1;
static const data_type EMPTY_ELEMENT = 0;

static list_return_e SetCanary(void* pointer, uint64_t value);
static list_return_e NumerizeElements(list_t list, size_t start_index);
static list_return_e IncreaseCapacity(list_t list);
static list_return_e ListDot(const list_t list, const char* current_time);

static list_return_e ListVerify(const list_t list);
#ifndef NDEBUG
#define VERIFY_RET(___X) do{list_return_e ___output = ListVerify(___X); if (___output != LIST_RETURN_SUCCESS) { return ___output;}} while(0)
#else
#define VERIFY_RET(X)
#endif

struct list_element_s
{
    data_type element;
    ssize_t next;
    ssize_t previous;
};

struct list_s
{
    uint64_t* canary_start;
    list_element_s* data;
    uint64_t* canary_end;
    ssize_t free;
    size_t elements_count;
    size_t elements_capacity;
    size_t real_size_in_bytes;
};



//====================== ACTS_WITH_LIST ===========================

list_return_e
InitList(list_t*  list,
         size_t   start_list_size)
{
    assert(list != nullptr);

    *list = (list_t) calloc(1, sizeof(list_s));

    if (*list == NULL)
    {
        return LIST_RETURN_ALLOCATION_ERROR;
    }

    (*list)->real_size_in_bytes = 2 * sizeof(uint64_t) * CANARY_SIZE
                               + start_list_size * sizeof(list_element_s);

    (*list)->canary_start = (uint64_t*) calloc((*list)->real_size_in_bytes, sizeof(uint8_t));

    if ((*list)->canary_start == NULL)
    {
        free(*list);

        return LIST_RETURN_ALLOCATION_ERROR;
    }

    SetCanary((*list)->canary_start, CANARY_FILL);

    (*list)->data = (list_element_s*) ((uint8_t*)(*list)->canary_start
                                       + CANARY_SIZE * sizeof(uint64_t));

    (*list)->free = 1;
    (*list)->elements_count = 0;
    (*list)->elements_capacity = start_list_size;

    ssize_t memory_alignment = (ssize_t) ((*list)->data + (*list)->elements_capacity);

    if ((memory_alignment & 0b00000111) != 0)
    {
        memory_alignment = ((memory_alignment >> 3) + 1) << 3;
    }

    SetCanary((void*) memory_alignment, CANARY_FILL);

    (*list)->canary_end = (uint64_t*) memory_alignment;

    NumerizeElements(*list, 1);

    ((*list)->data[0]).next = 0;
    ((*list)->data[0]).previous = 0;

    #ifndef NSHOWHISTORY
    ListDump(*list, "Init");
    #endif

    return LIST_RETURN_SUCCESS;
}

list_return_e 
ListInitNewElem(list_t    list,
                data_type value,
                size_t*   ind_ptr)
{
    assert(list != nullptr);
    assert(ind_ptr != nullptr);

    list_return_e output = LIST_RETURN_SUCCESS;
    if (list->elements_count >= list->elements_capacity - 2)
    {
        if ((output = IncreaseCapacity(list)) != LIST_RETURN_SUCCESS)
        {
            return output;
        }
    }

    list->data[list->free].element = value;
    list->data[list->free].previous = 0;
    *ind_ptr = (size_t) list->free;
    list->free = list->data[list->free].next;
    list->data[*ind_ptr].next = 0;

    list->elements_count++;

    #ifndef NSHOWHISTORY
    ListDump(list, "AddAfter");
    #endif

    return LIST_RETURN_SUCCESS;
}

list_return_e
ListAddAfterElement(list_t    list,
                    data_type value,
                    size_t    index)
{
    assert(list != NULL);
    VERIFY_RET(list);

    if ((index >= (size_t) list->elements_capacity)
        || (list->data[index].previous == NO_LINK))
    {
        return LIST_RETURN_INCORRECT_VALUE;
    }

    list_return_e output = LIST_RETURN_SUCCESS;
    if (list->elements_count >= list->elements_capacity - 2)
    {
        if ((output = IncreaseCapacity(list)) != LIST_RETURN_SUCCESS)
        {
            return output;
        }
    }

    ssize_t intermediate_value = 0;
    list->data[list->free].element = value;
    list->data[list->free].previous = (ssize_t) index;

    intermediate_value = list->data[index].next;
    list->data[index].next = list->free;
    list->data[intermediate_value].previous = list->free;
    list->free = list->data[list->free].next;

    list->data[list->data[index].next].next = intermediate_value;

    list->elements_count++;

    VERIFY_RET(list);

    #ifndef NSHOWHISTORY
    ListDump(list, "AddAfter");
    #endif

    return LIST_RETURN_SUCCESS;
}

list_return_e
ListDeleteElement(list_t list,
                  size_t index)
{
    assert(list != NULL);
    VERIFY_RET(list);

    if ((index >= list->elements_capacity)
        || (index == 0)
        || (list->data[index].previous == NO_LINK))
    {
        return LIST_RETURN_INCORRECT_VALUE;
    }

    list->data[list->data[index].previous].next = list->data[index].next;
    list->data[list->data[index].next].previous = list->data[index].previous;

    list->data[index].element = EMPTY_ELEMENT;
    list->data[index].previous = NO_LINK;
    list->data[index].next = list->free;
    list->free = (ssize_t) index;

    list->elements_count--;

    VERIFY_RET(list);

    #ifndef NSHOWHISTORY
    ListDump(list, "Delete");
    #endif

    return LIST_RETURN_SUCCESS;
}

list_return_e
DestroyList(list_t* list)
{
    if ((list != NULL) && (*list != NULL))
    {
        free((*list)->canary_start);
        free(*list);

        *list = NULL;
    }

    if (fclose(GetLogFile()) != 0)
    {
        return LIST_RETURN_FILE_CLOSE_ERROR;
    }

    return LIST_RETURN_SUCCESS;
}
// ================== NAVIGATION_IN_LIST =======================

list_return_e
GetElementValue(const list_t list,
                size_t       element_index,
                data_type*   value)
{
    assert(list != nullptr);
    assert(value != nullptr);
    VERIFY_RET(list);

     if ((element_index > list->elements_capacity)
        || (list->data[element_index].previous == NO_LINK)
        || (element_index == 0))
    {
        return LIST_RETURN_INCORRECT_VALUE;
    }

    *value = list->data[element_index].element;

    VERIFY_RET(list);

    return LIST_RETURN_SUCCESS;
}

ssize_t
GetNextElement(const list_t list,
               size_t       element_index)
{
    assert(list);
    VERIFY_RET(list);

    if ((element_index > list->elements_capacity)
        || (list->data[element_index].previous == NO_LINK)
        || (element_index == 0))
    {
        return NO_LINK;
    }

    VERIFY_RET(list);

    return list->data[element_index].next;
}

ssize_t
GetPreviousElement(const list_t list,
                   size_t       element_index)
{
    assert(list != nullptr);
    VERIFY_RET(list);

    if ((element_index > list->elements_capacity)
        || (list->data[element_index].previous == NO_LINK)
        || (element_index == 0))
    {
        return NO_LINK;
    }

    VERIFY_RET(list);

    return list->data[element_index].previous;
}

// ================== LOGGER =============================

static const char* LOG_FILE_NAME = "logs/log_file.htm";

static void PrintHTMLHeader(FILE* log_file, const char* current_time);
static void PrintListInfo(const list_t list, const char* current_time, FILE* log_file);
static void PrintElementsInfo(const list_t list, FILE* log_file);
static void PrintBytesInfo(const list_t list, FILE* log_file);

FILE*
GetLogFile()
{
    static FILE* log_file = fopen(LOG_FILE_NAME, "w+");
    return log_file;
}

void
SetLogFileName(const char* log_file_name)
{
    LOG_FILE_NAME = log_file_name;
}

list_return_e
ListDump(const list_t list,
         const char*  comment)
{
    assert(list != NULL);
    assert(comment != NULL);

    FILE* log_file = GetLogFile();
    if (log_file == NULL)
    {
        return LIST_RETURN_FILE_OPEN_ERROR;
    }

    const size_t str_time_size = 100;
    char current_time[str_time_size] = {};
    GetTime(current_time, str_time_size);

    PrintHTMLHeader(log_file, current_time);
    fprintf(log_file, "<h4>Comment:\"%s\"</h4>", comment);
    ListDot(list, current_time);
    PrintListInfo(list, current_time, log_file);
    PrintElementsInfo(list, log_file);
    PrintBytesInfo(list,log_file);

    return LIST_RETURN_SUCCESS;
}

// ================= LIST_VERIFY ======================

static bool CheckCanary(const list_t list);

static list_return_e
ListVerify(const list_t list)
{
    if ((list->canary_start == NULL)
        || (list->data == NULL)
        || (list->canary_end == NULL))
    {
        ListDump(list, "Null arg");
        return LIST_RETURN_NULL_POINTER;
    }
    else if (!CheckCanary(list))
    {
        ListDump(list, "Canary Error");
        return LIST_RETURN_CANARY_DAMAGED;
    }

    return LIST_RETURN_SUCCESS;
}

//============= HELPER_FUNCTIONS ======================

static list_return_e
IncreaseCapacity(list_t list)
{
    VERIFY_RET(list);

    SetCanary(list->canary_end, 0);
    (list->canary_start) = (uint64_t*) recalloc(list->canary_start,
                                                list->real_size_in_bytes,
                                                list->real_size_in_bytes
                                                + sizeof(list_element_s)
                                                * list->elements_capacity);

    if (list->canary_start == NULL)
    {
        return LIST_RETURN_ALLOCATION_ERROR;
    }

    list->real_size_in_bytes += sizeof(list_element_s) * list->elements_capacity;

    list->data = (list_element_s*) ((uint8_t*) list->canary_start
                                    + sizeof(uint64_t) * CANARY_SIZE);
    list->elements_capacity = list->elements_capacity << 1;

    NumerizeElements(list, list->elements_count + 1);

    list->free = (int) list->elements_count + 1;

    size_t memory_alignment = (size_t) (list->data + list->elements_capacity);

    if ((memory_alignment & 0b00000111) != 0)
    {
        memory_alignment = ((memory_alignment >> 3) + 1) << 3;
    }

    SetCanary((void*) memory_alignment, CANARY_FILL);

    list->canary_end = (uint64_t*) memory_alignment;

    VERIFY_RET(list);

    return LIST_RETURN_SUCCESS;
}

static list_return_e
SetCanary(void*    pointer,
          uint64_t value)
{
    for (ssize_t index = 0; index < CANARY_SIZE; index++)
    {
        ((uint64_t*) pointer)[index] = value;
    }

    return LIST_RETURN_SUCCESS;
}

static list_return_e
NumerizeElements(list_t list,
                 size_t start_index)
{
    for (size_t index = start_index; index < list->elements_capacity; index++)
    {
        (list->data[index]).next = (ssize_t) index + 1;
        (list->data[index]).previous = NO_LINK;
    }

    list->data[list->elements_capacity - 1].next = NO_LINK;

    return LIST_RETURN_SUCCESS;
}

static bool
CheckCanary(const list_t list)
{
    for (size_t index = 0; index < CANARY_SIZE; index++)
    {
        if ((((uint64_t*) list->canary_start)[index] != CANARY_FILL) ||
        (((uint64_t*) list->canary_end)[index] != CANARY_FILL))
        {
            return false;
        }
    }

    return true;
}

//================= BETTER_NOT_TO_WATCH ====================

static void DrawFilledElement(const list_t list, size_t  index, FILE* dot_file);
static void DrawEmptyElement(const list_t list, size_t  index, FILE* dot_file);
static void DrawInfoElements(const list_t list, FILE* dot_file);

static list_return_e
ListDot(const list_t list,
        const char*  current_time)
{
    assert(current_time != nullptr);

    const ssize_t max_string_size = 40;

    char name_template[max_string_size] = {};

    snprintf(name_template, max_string_size - 1, "logs/%s.gv", current_time);
    FILE* dot_file = fopen(name_template, "w+");

    if (dot_file == NULL)
    {
        return LIST_RETURN_FILE_OPEN_ERROR;
    }

    fprintf(dot_file, "graph  G{ bgcolor = \"#303030\";"
                      "splines = ortho; node [pin = \"true\", shape = box,"
                       "style = filled,]");

    for (size_t index = 1; index < list->elements_capacity; index++)
    {
        if  (list->data[index].previous != NO_LINK)
        {
            DrawFilledElement(list, index, dot_file);
        }
        else
        {
            DrawEmptyElement(list, index, dot_file);
        }
    }

    DrawInfoElements(list, dot_file);

    if (fclose(dot_file) != 0)
    {
        return LIST_RETURN_FILE_CLOSE_ERROR;
    }

    const ssize_t max_command_size = 200;
    char command[max_command_size] = {};

    snprintf(command, max_command_size - 1, "neato -Tpng logs/%s.gv -o"
             "logs/%s.png", current_time,
             current_time);

    system(command);

    return LIST_RETURN_SUCCESS;
}

// ======================= PRINT_INFO_FUNCTION ===================

static void
PrintHTMLHeader(FILE*       log_file,
                const char* current_time)
{
    assert(log_file != nullptr);
    assert(current_time != nullptr);

    fprintf(log_file, "<html>\n"
                        "<style>"
                        "body{background-color: rgb(48, 48, 48);}"
                        "h1{color: rgb(212, 58, 56);}"
                        "h2{color: rgba(153, 26, 24, 1);}"
                        "h4{color: rgb(182, 182, 182);}"
                        "</style>"
                        "<h1> LIST_DUMP %s</h1>\n",  current_time);
}

static void
PrintListInfo(const list_t list,
              const char*  current_time,
              FILE*        log_file)
{
    assert(list != nullptr);
    assert(current_time != nullptr);
    assert(log_file != nullptr);

    const ssize_t max_string_size = 50;
    char img_template[max_string_size] = {};
    snprintf(img_template, max_string_size - 1, "<img src=\"%s.png\","
                                                "height = \"20%%\">",
                                                current_time);
    fprintf(log_file, "%s", img_template);

    fprintf(log_file, "<p><h4>List element capacity:............................%zu<br/>",
            list->elements_capacity);
    fprintf(log_file, "List element count:.................................%zu<br/>",
            list->elements_count);
    fprintf(log_file, "List free element number:.....................%ld<br/>",
            list->free);
    fprintf(log_file, "List element capacity in bytes:..............%zu</h4>",
            list->real_size_in_bytes);
    fprintf(log_file,"<h2>LIST_ELEMENTS</h2>");
}

static void
PrintElementsInfo(const list_t list,
                  FILE*        log_file)
{
    assert(list != nullptr);
    assert(log_file != nullptr);

    for (size_t index = 0; index < list->elements_capacity; index++)
        {
            fprintf(log_file,
                    "<h4><p><li>index    = %4zu<br/>"
                    "value    = %4f<br/>"
                    "previous = %4ld<br/>"
                    "next     = %4ld<br/></p></li></h4>", index,
                    list->data[index].element,
                    list->data[index].previous, list->data[index].next);
        }
}

static void
PrintBytesInfo(const list_t list,
               FILE*        log_file)
{
    assert(list != nullptr);
    assert(log_file != nullptr);

    fprintf(log_file, "<h2>BYTE_LEGEND</h2><table style ="
                      "\"color:rgb(182, 182, 182);><tr>\"");

    for (size_t index = 0; index < list->real_size_in_bytes; index++)
    {
        if (index % 16 == 0)
        {
            fprintf(log_file, "</tr><tr><td><span style=\"color:"
                              "rgb(212, 58, 56)\"> %p:   </span></td>",
                              (uint8_t*) list->data + index);
        }

        if (((uint8_t*) list->canary_start)[index])
        {
            fprintf(log_file, "<td> <span style=\"color:"
                              "rgba(161, 24, 22, 1)\"> %02x </span></td>",
                    ((uint8_t*) list->canary_start)[index]);
        }
        else
        {
            fprintf(log_file, "<td> %02x </td>",
                              ((uint8_t*) list->canary_start)[index]);
        }

    }

    fprintf(log_file, "</tr></table>");
}

static void
DrawFilledElement(const list_t list,
                  size_t       index,
                  FILE*        dot_file)
{

    assert(list != nullptr);
    assert(dot_file != nullptr);

    fprintf(dot_file, "p%zu[ fillcolor = \"#949494\","
                        "label = \"prev = %ld\", width = 1.8"
                        ",pos = \"%zu.05, 10!\"];\n", index,
                        list->data[index].previous, 4 + 5 * index);

    fprintf(dot_file, "i%zu[fillcolor =\"#b6b6b6ff\","
                        "label = \"index = %zu\", width = 3.7,"
                        "pos = \"%zu, 11.2!\"];", index,
                        index, 5 + 5 * index);

    fprintf(dot_file, "v%zu[fillcolor =\"#b16261\","
                        "label = \"value = %f\"width = 3.7, "
                        "pos = \"%zu,10.6!\"];", index,
                        list->data[index].element, 5 + 5 * index);

    fprintf(dot_file, "n%zu[fillcolor =\"#949494\","
                        "label = \"next = %ld\", width = 1.8,"
                        " pos = \"%zu.95,10!\"];",
                        index, list->data[index].next,5 + 5 * index);

    fprintf(dot_file, "inv%zu[style=\"invis\","
                        "height = 2, pos = \"%zu.5, 11!\"];",
                        index, 7 + 5 * index);

    if ((list->data[index].next != 0) && (list->data[index].next != NO_LINK))
    {
        fprintf(dot_file, "n%zu -- p%ld[color = \"#d1d1d1\", dir = both];",
                index, list->data[index].next);
    }
}

static void
DrawEmptyElement(const list_t list,
                 size_t       index,
                 FILE*        dot_file)
{
    assert(dot_file != nullptr);

    fprintf(dot_file, "p%zu[fillcolor = \"#818181ff\","
                      "label = \"prev = %ld\", width = 1.8"
                      ",pos = \"%zu.05, 10!\"];", index,
                      list->data[index].previous, 4 + 5 * index);

    fprintf(dot_file, "i%zu[fillcolor =\"#818181ff\","
                      "label = \"index = %zu\","
                      "width = 3.7,pos = \"%zu, 11.2!\"];", index,
                      index, 5 + 5 * index);

    fprintf(dot_file, "v%zu[fillcolor =\"#818181ff\","
                      "label = \"value = %f\",width = 3.7,"
                      "pos = \"%zu,10.6!\"];", index,
                      list->data[index].element, 5 + 5 * index);

    fprintf(dot_file, "n%zu[fillcolor =\"#818181ff\","
                      "label = \"next = %ld\", width = 1.8,"
                      " pos = \"%zu.95,10!\"];", index,
                      list->data[index].next,5 + 5 * index);

    fprintf(dot_file, "inv%zu[style=\"invis\","
                      "height = 2, pos = \"%zu.5, 11!\"];",
                      index, 7 + 5 * index);

    if (list->data[index].next != NO_LINK)
    {
        fprintf(dot_file, "n%zu -- p%ld[color = \"#aaaaaa96\","
                "dir = forward];", index, list->data[index].next);
    }
}

static void
DrawInfoElements(const list_t list,
                 FILE*        dot_file)
{
    assert(list != nullptr);
    assert(dot_file != nullptr);

    fprintf(dot_file, "free[fillcolor = \"#646464ff\","
                      "label = \"free = %ld\""
                      ",width = 2, pos = \"%ld,13!\"];",
                      list->free, 5 + 5 * list->free);

    fprintf(dot_file, "free -- i%ld[color = \"#d1d1d1\",dir = forward];",
                      list->free);

    fprintf(dot_file, "}");
}

#undef LIST_VERIFY
