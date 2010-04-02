#ifndef CRIO_EVAL_H_
#define CRIO_EVAL_H_

#include <stdlib.h>
#include <stdio.h>
#include "crio.h"
#include "crio_mem.h"

enum crio_types {
    CRIO_INT_T,
    CRIO_FILTER_T,              /* crio filters defined by user */
    CRIO_FUN_T,                 /* basic ops: &, |, ! */
    CRIO_LIST_T                 /* pair-list */
};

typedef struct _crio_list {
    struct _crio_node *node;
    struct _crio_list *next;
} CrioList;

struct _crio_node {
    enum crio_types type;
    union {
        int (*fun)(struct _crio_list *, struct crio_stream *);
        struct crio_filter *filter;
        int boolean;
        CrioList *list;
    } value;
};

#define CRIO_TYPE(x) (x)->type
#define IS_CRIO_INT_T(x) CRIO_TYPE(x) == CRIO_INT_T
#define IS_CRIO_FILTER_T(x) CRIO_TYPE(x) == CRIO_FILTER_T
#define IS_CRIO_FUN_T(x) CRIO_TYPE(x) == CRIO_FUN_T
#define IS_CRIO_LIST_T(x) CRIO_TYPE(x) == CRIO_LIST_T

#define CRIO_FILTER(x) (x)->value.filter
#define CRIO_VALUE(x) (x)->value.boolean
#define CRIO_FUN(x) (x)->value.fun
#define CRIO_LIST(x) (x)->value.list

#define CRIO_CAR(x) (x)->node
#define CRIO_CDR(x) (x)->next

extern CrioList *LIST_NIL;
#define CRIO_IS_NIL(x) (((x) == LIST_NIL))

void crio_set_global_mem_pool(struct _crio_mpool *pool);
struct _crio_mpool * crio_get_global_mem_pool();


CrioList *crio_cons(CrioNode node, CrioList *list);
void crio_list_free(CrioList *list, int keep_nodes);
CrioList *crio_list_reverse(CrioList *list);

void crio_print_node(CrioNode node);
void crio_print_list(CrioList *list);


CrioNode crio_mknode_int(int v);

CrioNode crio_mknode_fun(int (*fun)(CrioList *, struct crio_stream *));

CrioNode
crio_mknode_filter(struct crio_filter *filter);

CrioNode
crio_mknode_list(CrioList *list);

CrioNode crio_mknode_fun_and();
CrioNode crio_mknode_fun_or();
CrioNode crio_mknode_fun_not();

CrioList *
crio_cons(CrioNode node, CrioList *list);

CrioNode
_crio_eval(CrioList *list, struct crio_stream *stream);



#define CRIO_DEBUG

#ifdef CRIO_DEBUG
  #define WHERESTR  "[file %s, line %d]: "
  #define WHEREARG  __FILE__, __LINE__
  #define DEBUGPRINT2(...) fprintf(stderr, __VA_ARGS__)
  #define DEBUGPRINT(_fmt, ...)  DEBUGPRINT2(WHERESTR _fmt, WHEREARG, __VA_ARGS__)
#endif

#endif  /* CRIO_EVAL_H_ */
