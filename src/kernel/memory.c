#include <memory.h>
#include <string.h>

static uint8_t heap_buffer[HEAP_SIZE]__attribute__((aligned(16)));

heap_block_t* heap_head = NULL;

void heap_init(void)
{
    my_memset(heap_buffer, 0, sizeof(heap_buffer));
    heap_head = (heap_block_t*)heap_buffer;
    heap_head->size = HEAP_SIZE - sizeof(heap_block_t);
    heap_head->is_free = 1;
    heap_head->next = NULL;
}

void* mem_allocate(size_t size)
{
    if (size == 0) return NULL;
    if (!heap_head) heap_init();
    // allign to 16 bytes
    size = (size + 15) & ~15;
    
    heap_block_t* current_heap_block = heap_head;
    while(current_heap_block){
        if (current_heap_block->is_free && current_heap_block->size >= size){
            if (current_heap_block->size >= size + sizeof(heap_block_t) + 16){
                heap_block_t* next_block = (heap_block_t*)((uint8_t*)current_heap_block + sizeof(heap_block_t) + size);
                next_block->size = current_heap_block->size - size - sizeof(heap_block_t);
                next_block->is_free = 1;
                next_block->next = current_heap_block->next;

                current_heap_block->size = size;
                current_heap_block->next = next_block;
            }
            current_heap_block->is_free = 0;
            void* ptr = (void*)((uint8_t*)current_heap_block + sizeof(heap_block_t));
            my_memset(ptr, 0, size);
            return ptr;
        }
        current_heap_block = current_heap_block->next;
    }
    return NULL;
}

void mem_free(void* ptr)
{
    if (!ptr) return;

    // Retrieve block header placed directly behind user payload
    heap_block_t* block = (heap_block_t*)((uint8_t*)ptr - sizeof(heap_block_t));
    block->is_free = 1;
    
    // Coalesce adjacent free blocks to prevent fragmentation
    heap_block_t* current = heap_head;
    while(current && current->next){
        if (current->is_free && current->next->is_free){
            current->size += sizeof(heap_block_t) + current->next->size;
            current->next = current->next->next;
        }else{
            current = current->next;
        }
    }
}
