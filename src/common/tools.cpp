#include "tools.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>

void* recalloc(void*  pointer,
               size_t current_size,
               size_t new_size)
{
    assert(pointer != nullptr);

    void* backup_pointer = NULL;

    backup_pointer = realloc(pointer, new_size);

    if (backup_pointer == NULL)
    {
        return NULL;
    }

    pointer = backup_pointer;

    memset((char*) pointer + current_size, 0, new_size - current_size);

    return pointer;
}

void
GetTime(char*        current_time,
        const size_t string_size)
{
    assert(current_time != nullptr);

    time_t t;
    struct tm *tmp;
    time(&t);
    char tmp_string[256] = {};
    tmp = localtime(&t);
    clock_t start_t = clock();
    strftime(tmp_string, string_size, "%Y-%m-%H-%M-%S", tmp);
    snprintf(current_time, string_size,"%s-%.0f", tmp_string,
            1000000000 * (((double) (start_t % CLOCKS_PER_SEC)) / CLOCKS_PER_SEC));
}

ssize_t 
GetFileSize(const char* file_name)
{
    assert(file_name != NULL);
    
    struct stat file_stat = {};

    if (stat(file_name, &file_stat) != 0)
    {
        return -1;
    }

    ssize_t char_number = file_stat.st_size;
    
    return char_number;
}
