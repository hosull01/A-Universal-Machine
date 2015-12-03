#include "umstack.h"
#include "assert.h"

struct Node {
    uint32_t segID;
    struct Node* link;
};

struct umStack {
    struct Node* head;
    uint32_t count;
};

/* returns a new empty stack */
umStack Stack_new()
{
    umStack stk = malloc(sizeof(*stk));
    //NEW(stk);
    
    stk->count = 0;
    stk->head = NULL;
    return stk;
}

/* returns 1 if stack is empty, 0 if otherwise */
inline uint32_t Stack_empty(umStack stk)
{
    assert(stk);
    return (stk->count == 0);
}

/* returns the last element pushed into the stack */
inline uint32_t Stack_pop(umStack stk)
{
    struct Node* popped_node;
    uint32_t result;
    
    assert (stk);
    assert (stk->count > 0);
    
    popped_node = stk->head;
    stk->head = popped_node->link;
    stk->count--;
    //popped_node->link = NULL;
    result = popped_node->segID;
    free(popped_node);
    return result;
}

/* adds an element into the stack */
inline void Stack_push(umStack stk, uint32_t id)
{
    struct Node* push_node = malloc(sizeof(*push_node));
    
    assert(stk);
    //NEW(push_node);
    push_node->segID = id;
    push_node->link = stk->head;
    stk->head = push_node;
    stk->count++;
}


/* deletes the stack */
void Stack_free(umStack *stk)
{
    struct Node *t, *u;
    
    assert(stk && *stk);
    for (t = (*stk)->head; t; t = u) {
        u = t->link;
        free(t);
    }
    free(*stk);
}
