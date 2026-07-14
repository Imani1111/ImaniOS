#include <string.h>

char* my_strcpy(char* dest, const char* src)
{
    char* saved_dest = dest;
    while(*src != '\0'){
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
    return saved_dest;
}

char* my_strcat(char* dest, const char* src)
{
    char* saved_dest = dest;
    while(*dest != '\0'){
        dest++;
    }

    while(*src != '\0'){
        *dest = *src;
        src++;
        dest++;
    }
    *dest = '\0';
    return saved_dest;
}

int my_strcmp(const char* s1, const char* s2)
{
    while (*s1 && (*s1 == *s2)){
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

void* my_memcpy(void* restrict dest, void* restrict src, size_t num)
{
    uint8_t* s = (uint8_t*)src;
    uint8_t* d = (uint8_t*)dest;

    for (size_t i = 0; i < num; i++){
        d[i] = s[i];
    }
    return dest;
}

void* my_memset(void* dest, int ch, size_t count)
{
    uint8_t* ptr = (uint8_t*)dest;
    uint8_t value = (uint8_t)ch;

    while(count > 0 && ((uintptr_t)ptr & (sizeof(size_t) - 1)) != 0){
        *ptr++ = value;
        count--;
    }
    size_t word_value = value;
    for (size_t i = 1; i < (sizeof(size_t)); i++){
        word_value = (word_value << 8) | value;
    }

    size_t* word_ptr = (size_t*)ptr;
    while(count >= sizeof(size_t)){
        *word_ptr = word_value;
        count -= sizeof(size_t);
    }
    ptr = (uint8_t*)word_ptr;
    while(count > 0){
        *ptr = value;
        count--;
    }
    return dest;
}

uint32_t my_strlen(char* string)
{
    uint32_t len = 0;
    while (string[len] != '\0'){
        len++;
    }
    return len;
}

char* my_strncpy(char* dest, const char* src, size_t count)
{
    uint8_t* temp = (uint8_t*)dest;

    while (count > 0 && *src != '\0'){
        *temp++ = *src++;
        count--;
    }
    while (count > 0){
        *temp++ = '\0';
        count--;
    }

    return dest;
}

void parse_cmd(char* input, char* command, char* args)
{
    int i = 0;
    int command_index = 0;
    int args_index = 0;

    while (input[i] == ' '){
        i++;
    }

    while(input[i] != ' ' && input[i] != '\0'){
        command[command_index++] = input[i++];
    }
    command[command_index] = '\0';
    
    while(input[i] == ' '){
        i++;
    }

    while(input[i] != '\0'){
        args[args_index++] = input[i++];
    }
    args[args_index] = '\0';
}
