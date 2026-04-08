#include <hashtable.h>


// ================== LOGGER =============================

static const char* LOG_FILE_NAME = "logs/log_file.htm";

static void PrintHTMLHeader(FILE* log_file, const char* current_time);

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

    return LIST_RETURN_SUCCESS;
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

    const ssize_t max_string_size = 50;
    char img_template[max_string_size] = {};
    snprintf(img_template, max_string_size - 1, "<img src=\"%s.png\","
                                                "height = \"20%%\">",
                                                current_time);
    fprintf(log_file, "%s", img_template);
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

