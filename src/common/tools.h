#ifndef TOOLS_H
#define TOOLS_H

#include <stdlib.h>

ssize_t GetFileSize(const char* file_name);
void* recalloc(void*  pointer, size_t current_size, size_t new_size);
void  GetTime(char*  current_time, const size_t string_size);
extern "C" int ssestrncmp(const char* s1, const char* s2, int n);

#endif //TOOLS_H
