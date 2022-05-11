#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <string.h>
#include "memAlloc.h"
#include "stack.h"
#include "doctest.h"

/**
 * free the allocation is not needed in these tests
 * because the os will free them anyway when the test will finished.
 */

TEST_CASE ("memAlloc testing") {
    SUBCASE("block split and correct size allocate"){
        int *ptr = (int *) my_malloc(30 * sizeof (int));
        block block120 = (block) ptr - 1;
        CHECK_EQ(block120->size, 120); // sizeof(int) * 30
        my_free(ptr);
        (int *) my_malloc(20 * sizeof (int));
        // check if it split the block120 to two blocks of 80 and (40 - size of block)
        CHECK_EQ(block120->size, 80); // sizeof(int) * 20
        CHECK_EQ(block120->next->size, 40 - BLOCK_SIZE); // minus BLOCK_SIZE because its also create a new block when splitting
    }

    SUBCASE("malloc using the same address for the same size"){
        int *ptr = (int *) my_malloc(sizeof (int));
        int* saved_ptr = ptr;
        my_free(ptr);
        ptr = (int *) my_malloc(sizeof (int));
        CHECK_EQ(saved_ptr, ptr); // should be the same
    }

    SUBCASE("malloc using the same address for a smaller but closest size to the requested size"){
        // This case check how we allocate 12 bytes when there is all ready 20 bytes free
        // it spouse to take the same pointer allocate for the 20 bytes because this is the block with the closest size
        // to the requested size. (it also splits the block but this is check in other subcase)
        int *ptr = (int *) my_malloc(5 * sizeof (int));
        int* saved_ptr = ptr;
        my_free(ptr);
        ptr = (int *) my_malloc(3 * sizeof (int));
        CHECK_EQ(saved_ptr, ptr); // should be the same
    }

    SUBCASE("calloc ZERO") {
        int *ptr = (int *) my_calloc(1, sizeof(int));
        CHECK(*ptr == 0);
    }
}

TEST_CASE ("stack testing") {

    //   The function strcmp return 0 when they much and 1 otherwise

    char * test1 = (char *) my_malloc(5);// must be dynamic because pop call free on the string inside the stack node
    char * test2 = (char *) my_malloc(5);

    test1[0] = 't', test1[1] = 'e', test1[2] = 's', test1[3] = 't', test1[4] = '1';
    test2[0] = 't', test2[1] = 'e', test2[2] = 's', test2[3] = 't', test2[4] = '2';

    stack st; // for stack tests

    push(&st, test1);
    CHECK(strncmp(top(&st), "test1", 5) == 0);

    push(&st, test2);
    CHECK(strncmp(top(&st), "test2", 5) == 0);

    pop(&st);
    CHECK(strncmp(top(&st), "test1", 5) == 0);

}

