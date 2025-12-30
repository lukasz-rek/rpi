#include "heap.h"
#include "mm.h"
#include "printf.h"

#define HEAP_DATA_LOC(block_ptr) ((char*)(block_ptr) + sizeof(heap_block_t))


static unsigned long heap_start;
static unsigned long heap_end;
static heap_block_t* first_block;

void heap_init() {
    // We need to request 256 consecutive pages
    // This needs to be called before scheduler pops off

    heap_start = allocate_kernel_page();
    printf("Heap start allocated at: 0x%x\n", heap_start);
    for (int i = 1; i < 256; i++) {
        // Nothing else requests pages so are next to each other
        heap_end = allocate_kernel_page();
    }
    printf("Heap end allocated at: 0x%x\n", heap_end);

    first_block = (heap_block_t *)heap_start;
    first_block->is_used = 0;
    first_block->next_block = 0;
    first_block->prev_block = 0;
}

void print_heap_state(){
    heap_block_t *free_block_ptr = first_block;
    while(free_block_ptr != 0){
        printf("Block at addr: 0x%lx, data: 0x%lx, used: %d, next: 0x%lx, prev: 0x%lx\n", free_block_ptr, HEAP_DATA_LOC(free_block_ptr),free_block_ptr->is_used, free_block_ptr->next_block, free_block_ptr->prev_block);
        free_block_ptr = free_block_ptr->next_block;
    }
    printf("Done\n");
}

/*
------------------------------------------
| heap_block_t | some mem | heap_block_t |
------------------------------------------
*/

void *kmalloc(unsigned long size) {
    // print_heap_state();
    // Check block by block until we find a free one or last
    heap_block_t *free_block_ptr = first_block;
    while (free_block_ptr != 0) {
        if (!free_block_ptr->is_used) {
            // It has nothing after it or enough space before next
            if (free_block_ptr->next_block == 0 || (HEAP_DATA_LOC(free_block_ptr) + size) <= free_block_ptr->next_block)
                break;
        } else
            free_block_ptr = free_block_ptr->next_block;
    }
    // We should either get nothing or block
    if (free_block_ptr == 0)
        return 0; // We have nothing

    heap_block_t* next_block = (heap_block_t*)((char*) free_block_ptr + sizeof(heap_block_t) + size);
    if(free_block_ptr->next_block == (HEAP_DATA_LOC(free_block_ptr) + size)) {
        // If the space is just enough for this allocation we don't need to create anything new
        free_block_ptr->is_used = 1;

    } else {
        // We need to create a new block and split up the free space
        free_block_ptr->is_used = 1;
        next_block->is_used = 0;
        next_block->next_block = free_block_ptr->next_block; // If it was nothing, it'll still be nothing
        next_block->prev_block = free_block_ptr;
        free_block_ptr->next_block = next_block;
    }
    // print_heap_state();
    return HEAP_DATA_LOC(free_block_ptr);

}

void kfree(void *ptr) {
    // print_heap_state();
    //TODO: check if this works fully, I don't use it much yet
    // See where it points to
    heap_block_t* block = (heap_block_t*)((char*)ptr - sizeof(heap_block_t));

    if (!block->is_used)
        return; // Nothing to free

    // If prev is also free, merge with ours
    if (block->prev_block != 0 && !block->prev_block->is_used) {
        heap_block_t* prev = block->prev_block;
        prev->next_block = block->next_block;
        block = prev;
    }
    // If next is free, merge with ours
    if (block->next_block != 0 && !block->next_block->is_used) {
        heap_block_t* next = block->next_block;
        block->next_block = next->next_block;
    }
    // Ensure it is free and we're done
    block->is_used = 0;
    // print_heap_state();
}
