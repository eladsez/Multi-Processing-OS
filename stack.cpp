/**
 * A basic string stack implementation representing by linked list
 */

#include <stdio.h>
#include "stack.h"
#include "memAlloc.h"

int push(stack *st, char *str) {
    node *toPush = (node *) my_malloc(sizeof(node));
    if (toPush == NULL) {
        perror("malloc in push to stack");
        return 0;
    }
    toPush->str = str;
    toPush->prev = st->top;
    st->top = toPush;
    st->size += 1;
    return 1;
}

void pop(stack *st) {
    if (st->size == 0)
        return;
    node* toTop = st->top->prev;
    my_free(st->top->str);
    my_free(st->top);
    st->top = toTop;
    st->size -= 1;
}

char* top(stack *st) {
    if (st->size == 0)
        return NULL;
    return st->top->str;
}

void freeStack(stack *st){
    while (st->size != 0){
        pop(st);
    }
}



