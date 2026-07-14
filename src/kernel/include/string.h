#ifndef STRING_H
#define STRING_H

#include <stdint.h>
#include <stddef.h>

char* my_strcpy(char* dest, const char* src);
char* my_strcat(char* dest, const char* src);
int my_strcmp(const char* s1, const char* s2);
void parse_cmd(char* buffer, char* command, char* args);
void* my_memcpy(void* dest, void* src, size_t num);
uint32_t my_strlen(char* string);

void* my_memset(void* dest, int ch, size_t count);
char* my_strncpy(char* dest, const char* src, size_t count);


#endif
