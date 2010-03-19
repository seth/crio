#ifndef CRIO_EVAL_H_
#define CRIO_EVAL_H_

#include <stdlib.h>
#include "crio.h"

enum crio_types {
    CRIO_INT_T,
    CRIO_FILTER_T,
    CRIO_FUN_T
};

typedef struct _crio_list {
    struct _crio_node *node;
    struct _crio_list *next;
} CrioList;

typedef struct _crio_node {
    enum crio_types type;
    union {
        struct crio_filter *filter;
        int boolean;
        int (*fun)(struct _crio_list *);
    } value;
} CrioNode;


#define CRIO_TYPE(x) (x)->type
#define IS_CRIO_INT_T(x) CRIO_TYPE(x) == CRIO_INT_T
#define IS_CRIO_FILTER_T(x) CRIO_TYPE(x) == CRIO_FILTER_T
#define IS_CRIO_FUN_T(x) CRIO_TYPE(x) == CRIO_FUN_T

#define CRIO_FILTER(x) (x)->value.filter
#define CRIO_VALUE(x) (x)->value.boolean
#define CRIO_FUN(x) (x)->value.fun

#define CRIO_CAR(x) (x)->node
#define CRIO_CDR(x) (x)->next


CrioList * crio_cons(CrioNode *node, CrioList *list);



#endif  /* CRIO_EVAL_H_ */
