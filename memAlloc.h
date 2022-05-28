#include <stddef.h>
#include "stack.h"

#ifndef MEMALLOC_HPP
#define MEMALLOC_HPP

#define BLOCK_SIZE sizeof(struct memory_block)

//struct used for the implementation of malloc, calloc and free
//Some kind of Two-way linked list
struct memory_block {
    size_t size; //size of the block
    struct memory_block * next; //next meta-data block
    struct memory_block * prev; //previous meta-data block
    int free; //this is a flag variable describing whether the block is free or not denoted by 0 and 1
};

typedef struct memory_block * block;

// init the process memory
void init_mem();

// init the pointer of the stack on the program break and increment him
void init_stack_mem(stack **st);

//Searching the Best fit memory allocation
block find_best_block(block * last, size_t size);

//In case there is no feet block create a new one
block new_space(block last, size_t size);

//If good block is found but not exactly feet split it into two
void split_block(block mblock, size_t size);

//Used to merge adjacency blocks when freeing
void coalesce(block my_block);

void *my_malloc(size_t size);
void *my_calloc(size_t nitems, size_t size);
void my_free(void *ptr);
void free_all();
#endif
