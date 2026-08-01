#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>

#define HEAP_SIZE 1048576

typedef struct HeapBlock{
    size_t size;
    int is_free;
    struct HeapBlock* next;
    uint32_t _padding;
}heap_block_t;

extern heap_block_t* heap_head;

void heap_init(void);
void* mem_allocate(size_t size);
void mem_free(void* ptr);

#endif
