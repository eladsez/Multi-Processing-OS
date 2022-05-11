#include <stdio.h>
#include <stddef.h>
#include <sys/mman.h>
#include "memAlloc.h"
#include "stack.h"

#define MB 1048576

void *headB = NULL; ///head of our linked list
void *prog_break = NULL; /// pointer to the program break

void init_mem(stack **st){
    prog_break = mmap(NULL, MB * 10, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (prog_break == -1){
        perror("mmap failed");
    }
    *st = (stack *)prog_break;
    prog_break += sizeof (stack);
}


/**
 * This function allocate a new memory on the heap with sbrk() system call
 * @param last The last block in the memory block linked list
 * @param size the size we wish to allocate
 * @return a block struct representing the new memory
 */
block new_space(block last, size_t size) {
    block new_block;
    new_block = (block) prog_break; //current top of the heap
    prog_break += size + BLOCK_SIZE;
    if (new_space == (void *) -1) {
        return NULL; //sbrk has failed
    }

    if (last) { //when last is not NULL, i.e this is not the first call for malloc. We add a new block at the end of the last block
        last->next = new_block;
    }

    new_block->size = size;
    new_block->free = 0;
    new_block->next = NULL; //End of the list
    new_block->prev = last; //assigning the previous ptr for the new block requested

    return new_block;

}


/**
 * @param last block in the memory block linked list
 * @param size the size we wish to allocate
 * @return the best fit block for the requested size and NULL if it didnt find
 */
block find_best_block(block *last, size_t size) {

    //Find the optimum block first
    block best_block = NULL;
    block curr = (block) headB;
    while (curr != NULL) {
        if ((curr->free && (curr->size >= size)) && (best_block == NULL || curr->size < best_block->size)) {
            best_block = curr; //assign the best block
            if (best_block->size == size) //break for optimization
                break;
        }

        curr = curr->next; //normal iteration
    }

    //Now assign the last block for requesting space in later situation
    curr = (block) headB;
    //This loop is to assign the last node for requesting space
    while (curr != NULL) {

        if (((best_block - curr) == 0))
            return curr;
        *last = curr; //Assigned the last node if we haven't found a best block
        curr = curr->next;
    }

    return curr; //may return NULL
}


/**
 * If we find a free block which exactly fits the required size, we don't need to do splitting.
 * Otherwise, if the size of the block we found is greater than the size of the requested slot,
 * it's better to split the block into two partitions
 * @param to_split the block we want to split
 * @param size the size we actually want to allocate
 */

void split_block(block to_split, size_t size) {

    if (to_split) { //if to_split is not NULL

        block new_block = (block) (void *) ((void *) to_split + size + BLOCK_SIZE);
        new_block->size = (to_split->size - size - BLOCK_SIZE);
        new_block->next = to_split->next;
        new_block->free = 1; //The new block is free
        new_block->prev = to_split;

        to_split->size = size; //The size of the input block gets reduced to the requested allocate size
        to_split->free = 0; //allocated
        to_split->next = new_block;

        if (new_block->next) { // in case the is another block after to_split block
            new_block->next->prev = new_block;
        }
    }
}

/**
 * Combine free blocks of adjacent memory into a single memory chunk
 * Initially tried differed coalescing, but it took a long time
 * After that switched to immediate coalescing
 * @param my_block
 */

void coalesce(block my_block) {
    //The following two cases would be able to handle all merge conditions

    if (my_block->next) {
        if (my_block->next == (block) 0x1) {
            return;
        }
        if (my_block->next->free) { //if the next block is free, merge the current block with the next block
            my_block->size += BLOCK_SIZE + my_block->next->size;
            my_block->next = my_block->next->next;

            if (my_block->next) {
                my_block->next->prev = my_block;
            }
        }
    }

    if (my_block->prev) {
        block temp;
        if (my_block->prev->free) { //if the previous blocks is free, merge with the previous block
            temp = my_block->prev;
            temp->size = temp->size + BLOCK_SIZE + my_block->size;
            temp->next = my_block->next;
            if (temp->next) {
                temp->next->prev = temp;
            }
        }
    }
}


/**
 * The implementation of malloc.
 * @param size the size we wish to allocate.
 * @return a void pointer of the allocated size request, NULL if didnt succeed.
 */
void *my_malloc(size_t size) {

    if (size <= 0) { //Requested size is less than zero
        return NULL;
    } else { //Size is greater than zero

        block my_block;
        block last_block;

        if (headB == NULL) { //We are calling malloc for the first time, the head of the Linked List is NULL

            my_block = new_space(NULL, size); //Request new space
            if (!my_block) { //if my_block is NULL
                return NULL;

            } else { //my_block is not NULL
                headB = my_block; //Assign the headB of the linked list
                return my_block + 1;
            }

        } else { //head is not NULL, malloc has been used at least once

            last_block = (block) headB;
            my_block = find_best_block(&last_block, size); //Search for the free block of memory
            if (my_block != NULL) { //suitable block is found
                if (my_block->size > size) { //If only the size of the block found is greater than the requirement we call the split_block function
                    split_block(my_block, size);
                    my_block->free = 0;
                }
                return my_block + 1;
            } else { //The case where no free block was found

                my_block = new_space(last_block, size);
                if (!my_block) {
                    return NULL;
                } else {
                    return my_block + 1;
                }
            }
        }
    }
}

/**
 * The implementation of calloc.
 * @param nitems This is the number of elements to be allocated.
 * @param size This is the size per element.
 * @return a void pointer of the allocated size request, NULL if didnt succeed.
 */
void *my_calloc(size_t nitems, size_t size) {
    void *ptr = my_malloc(nitems * size);
    if (ptr == NULL) {
        return NULL;
    }
    int *zero;
    for (int i = 0; i < nitems * size; ++i) {
        zero = (int *) (ptr + i);
        *zero = 0;
    }
    return ptr;
}


/**
 * The implementation of free.
 * @param ptr the pointer we wish to free
 */
void my_free(void *ptr) {

    if (!ptr) {
        //printf("Cannot free a null pointer.");
        return;
    } else { //valid ptr

        block block_ptr = (block) ptr - 1; // getting the block associated with ptr pointer
        if (block_ptr) {
            block_ptr->free = 1; //free the block
            coalesce(block_ptr); //merge the blocks
            return;
        } else { //when the block is NULL
            return;
        }
    }
}

void free_all(){
    size_t total_size = 0;
    block curr = (block) headB;
    while (curr != NULL){
        total_size += BLOCK_SIZE + curr->size;
        curr = curr->next;
    }
    munmap(headB, total_size);
}