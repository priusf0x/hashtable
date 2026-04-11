#include "buffer.h"

#include <assert.h>
#include <cctype>
#include <cstddef>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include <ctype.h>

#include "tools.h"

// ============================ MEMORY_CONTROLLING ============================

buffer_return_e
BufferCtor(buffer_t*    buffer,
           const char*  file_name)
{
    assert(buffer != NULL);
    assert(file_name != NULL);

    *buffer = (buffer_t) calloc (1, sizeof(buffer_s));

    if(*buffer == NULL)
    {
        return BUFFER_RETURN_ALLOCATION_ERROR;
    }

    ssize_t char_number = GetFileSize(file_name);

    if (char_number == -1)
    {
        free(*buffer);
        *buffer = NULL;
        
        return BUFFER_RETURN_SYSTEM_ERROR;
    }
    
    FILE* file_input = fopen(file_name , "r");
    if (file_input == NULL)
    {
        free(*buffer);
        *buffer = NULL;
    
        return BUFFER_RETUNR_FILE_OPEN_ERROR;
    }

    (*buffer)->buffer = (char*) calloc((size_t) char_number + 1, sizeof(char));
    if ((*buffer)->buffer == NULL)
    {
        free(*buffer);
        *buffer = NULL;
        fclose(file_input);
    
        return BUFFER_RETURN_ALLOCATION_ERROR;
    }
    
    size_t read_count = fread((*buffer)->buffer , sizeof(char), 
                                (size_t) char_number , file_input);
    
    if (fclose(file_input) != 0)
    {
        free((*buffer)->buffer);
        free(*buffer);
        *buffer = NULL;

        return BUFFER_RETURN_FILE_CLOSE_ERROR;
    }

    if (read_count == 0)
    {
        free((*buffer)->buffer);
        free(*buffer);
        *buffer = NULL;

        return BUFFER_RETURN_EMPTY_FILE;
    }
    
    (*buffer)->buffer[read_count] = 0;

    return BUFFER_RETURN_SUCCESS;
}

buffer_return_e
BufferDtor(buffer_t buffer)
{
    if (buffer != nullptr)
    {
        free(buffer->buffer);
        free(buffer);
    }

    return BUFFER_RETURN_SUCCESS;
}

// ================================ STRING_F ===================================

static size_t
SkipAlpha(const char* string,  
          size_t      current_position)
{
    assert(string != nullptr);

    char character = *(string + current_position);

    while (isalnum(character) && (character != '\0'))
    {
        current_position++;
        character = *(string + current_position);
    }

    return current_position;
}

static size_t
SkipNotAlpha(const char* string,
             size_t      current_position)
{
    assert(string != NULL);

    char character = *(string + current_position);

    while (!isalnum(character) && (character != '\0'))
    {
        current_position++;
        character = *(string + current_position);
    }

    return current_position;
}

// ================================ METHODS ===================================

void 
SkipAlphaB(buffer_t buffer)
{
    assert(buffer != nullptr);

    buffer->cur_pos = SkipAlpha(buffer->buffer, buffer->cur_pos);
}

void 
SkipNotAlphaB(buffer_t buffer)
{
    assert(buffer != nullptr);

    buffer->cur_pos = SkipNotAlpha(buffer->buffer, buffer->cur_pos);
}

void 
SkipNSymbolsB(buffer_t buffer,
              size_t   n)
{
    assert(buffer != nullptr);

    buffer->cur_pos += n;
}
