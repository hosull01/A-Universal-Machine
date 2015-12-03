#ifndef umstack_h
#define umstack_h

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


typedef struct umStack *umStack;

/* returns 1 if stack is empty, 0 if otherwise */
extern uint32_t Stack_empty(umStack stk);

/* deletes the stack */
extern void Stack_free(umStack *stk);

/* returns a new empty stack */
umStack Stack_new();

/* returns the last element pushed into the stack */
uint32_t Stack_pop(umStack stk);

/* adds an element into the stack */
void Stack_push(umStack stk, uint32_t);

#endif