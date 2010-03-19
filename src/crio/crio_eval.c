#include "crio_eval.h"

CrioList *
crio_cons(CrioNode *node, CrioList *list)
{
    int alloced_head = 0;
    CrioList *head = list;
    if (!list) {
        head = malloc(sizeof(CrioList));
        if (!head) return NULL;
        alloced_head = 1;
        head->next = NULL;
        head->node = NULL;
    }
    CrioList *e = malloc(sizeof(CrioList));
    if (!e) {
        if (alloced_head) free(head);
        return NULL;
    }
    e->node = (CrioNode *)node;
    e->next = NULL;
    return e;
}




