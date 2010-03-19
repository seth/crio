#include "crio_eval.h"


char *crio_type_names[3] = {"CRIO_INT_T",
                            "CRIO_FILTER_T",
                            "CRIO_FUN_T"};

CrioList _LIST_NIL = {NULL, NULL};
CrioList *LIST_NIL = &_LIST_NIL;

void crio_print_node(CrioNode *node)
{
    uintptr_t v = 0;

    switch (CRIO_TYPE(node)) {
    case CRIO_INT_T:
        v = (uintptr_t)CRIO_VALUE(node);
        break;
    case CRIO_FUN_T:
        v = (uintptr_t)CRIO_FUN(node);
        break;
    case CRIO_FILTER_T:
        v = (uintptr_t)CRIO_FILTER(node);
        break;
    default:
        v = 99999;                /* indicate bogus type */
        break;
    }
    printf("[%s: %lu]", crio_type_names[CRIO_TYPE(node)], v);
}

void crio_print_list(CrioList *list)
{
    if (!list) {
        printf("()\n");
        return;
    }
    CrioList *h = list;
    printf("(");
    while (1) {
        crio_print_node(CRIO_CAR(h));
        h = CRIO_CDR(h);
        if (!CRIO_IS_NIL(h)) printf(", "); else break;
    }
    printf(")\n");
}

CrioNode *
crio_mknode_int(int v)
{
    CrioNode *node = malloc(sizeof(CrioNode));
    if (!node) return NULL;
    CRIO_TYPE(node) = CRIO_INT_T;
    CRIO_VALUE(node) = v;
    return node;
}

CrioNode *
crio_mknode_fun(int (*fun)(CrioList *))
{
    CrioNode *node = malloc(sizeof(CrioNode));
    if (!node) return NULL;
    CRIO_TYPE(node) = CRIO_FUN_T;
    CRIO_FUN(node) = fun;
    return node;
}

CrioNode *
crio_mknode_filter(struct crio_filter *filter)
{
    CrioNode *node = malloc(sizeof(CrioNode));
    if (!node) return NULL;
    CRIO_TYPE(node) = CRIO_FILTER_T;
    CRIO_FILTER(node) = filter;
    return node;
}

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
    e->next = head;
    return e;
}

void crio_list_free(CrioList *list)
{
    CrioList *h = list, *tmp;
    if (list) {
        while (!CRIO_IS_NIL(h)) {
            tmp = CRIO_CDR(h);
            free(h);
            h = tmp;
        }
        if (h) free(h);
        list = NULL;
    }
}

CrioList *crio_list_reverse(CrioList *list)
{
    /* for now, creates a completely new list, no
       shared cons cells
     */
    CrioList *ans = NULL, *h = list;
    while (!CRIO_IS_NIL(h)) {
        ans = crio_cons(CRIO_CAR(h), ans);
        h = CRIO_CDR(h);
    }
    return ans;
}


CrioNode *
crio_eval_filter(CrioNode *e,
                 struct crio_stream *stream)
{
    struct crio_filter *cf = CRIO_FILTER(e);
    int res = cf->filter(stream, cf->filter_ctx);
    CrioNode *node = malloc(sizeof(CrioNode));
    if (!node) return NULL;
    CRIO_TYPE(node) = CRIO_INT_T;
    CRIO_VALUE(node) = res;
    return node;
}

CrioNode *
crio_eval_fun(CrioNode *e, CrioList *args)
{
    int res = CRIO_FUN(e)(args);
    CrioNode *node = malloc(sizeof(CrioNode));
    if (!node) return NULL;
    CRIO_TYPE(node) = CRIO_INT_T;
    CRIO_VALUE(node) = res;
    return node;
}

static int crio_fun_and(CrioList *args)
{
    /* for now, assume args are already evaluated
       and are valid CRIO_INT_T nodes.
     */
    CrioList *a = args;
    CrioNode *n;
    int ans = 1;
    while (!CRIO_IS_NIL(a)) {
        n = CRIO_CAR(a);
        ans = ans && CRIO_VALUE(n);
        if (!ans) break;
        a = CRIO_CDR(a);
    }
    return ans;
}

static int crio_fun_or(CrioList *args)
{
    /* for now, assume args are already evaluated
       and are valid CRIO_INT_T nodes.
     */
    CrioList *a = args;
    CrioNode *n;
    int ans = 0;
    while (!CRIO_IS_NIL(a)) {
        n = CRIO_CAR(a);
        ans = ans || CRIO_VALUE(n);
        if (ans) break;
        a = CRIO_CDR(a);
    }
    return ans;
}

CrioNode * crio_mknode_fun_and()
{
    return crio_mknode_fun(crio_fun_and);
}

CrioNode * crio_mknode_fun_or()
{
    return crio_mknode_fun(crio_fun_or);
}


CrioNode *
_crio_eval(CrioList *list, struct crio_stream *stream)
{
    CrioList *e = list;
    CrioNode *v = CRIO_CAR(list);
    CrioList *args = NULL;
    CrioList *args0 = NULL;

    switch (CRIO_TYPE(v)) {
    case CRIO_INT_T:
        return v;
    case CRIO_FUN_T:
        args = NULL;
        args0 = CRIO_CDR(e);
        while (!CRIO_IS_NIL(args0)) {
            CrioNode *a = _crio_eval(args0, stream);
            args = crio_cons(a, args);
            args0 = CRIO_CDR(args0);
        }
        return crio_eval_fun(v, args);
    case CRIO_FILTER_T:
        return crio_eval_filter(v, stream);
    }
    return NULL;                /* should not happen */
}