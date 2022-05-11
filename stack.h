#ifndef STACK_H
#define STACK_H

// node of the stack
struct node {
    struct node *prev;
    char *str; // pointer to the first char of the current string
};

typedef struct node node;

typedef struct stack {
    size_t size; // curr size of the stack
    node *top; // the top node
} stack;

int push(stack *st, char *str); // return: 0 when error accrued, 1 otherwise
void pop(stack *st);
char *top(stack *st);
void freeStack(stack *st);

#endif
