#include "crio_eval.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

static struct _crio_mpool *GLOBAL_MEM_POOL = NULL;

void crio_set_global_mem_pool(struct _crio_mpool *pool)
{
    GLOBAL_MEM_POOL = pool;
}

struct _crio_mpool *
crio_get_global_mem_pool()
{
    return GLOBAL_MEM_POOL;
}

static void *
xmalloc(size_t s)
{
    if (GLOBAL_MEM_POOL)
        return crio_mpool_alloc(GLOBAL_MEM_POOL, s);
    else {
        return malloc(s);
    }
}

static int crio_fun_and(CrioList *args, struct crio_stream *stream);
static int crio_fun_or(CrioList *args, struct crio_stream *stream);
static int crio_fun_not(CrioList *args, struct crio_stream *stream);

char *crio_type_names[3] = {"CRIO_INT_T",
                            "CRIO_FILTER_T",
                            "CRIO_FUN_T"};

CrioList _LIST_NIL = {NULL, NULL};
CrioList *LIST_NIL = &_LIST_NIL;

struct _crio_node FUN_NODE_AND = {CRIO_FUN_T, {crio_fun_and}};
struct _crio_node FUN_NODE_OR = {CRIO_FUN_T, {crio_fun_or}};
struct _crio_node FUN_NODE_NOT = {CRIO_FUN_T, {crio_fun_not}};

#define P2I(v) (intptr_t)v & 0xffffffff

int crio_node2str(CrioNode node, char *nbuf, size_t n)
{
    int len = 0;
    char name[50], buf[80];
    name[0] = 0;
    switch (CRIO_TYPE(node)) {
    case CRIO_INT_T:
        snprintf(buf, 80, "[INT (%d)]", CRIO_VALUE(node));
        break;
    case CRIO_FUN_T:
        if (CRIO_FUN(node) == &crio_fun_and)
            strncpy(name, "&", sizeof(name));
        else if (CRIO_FUN(node) == &crio_fun_or)
            strncpy(name, "|", sizeof(name));
        snprintf(buf, 80, "[FUN (%s)]", name);
        break;
    case CRIO_FILTER_T:
        snprintf(buf, 80, "[FILTER (%s)]", CRIO_FILTER(node)->name);
        break;
    case CRIO_LIST_T:
        return crio_list2str(CRIO_LIST(node), nbuf, n);
    default:
        snprintf(buf, 80, "[unknown: %d]", CRIO_TYPE(node));
        break;
    }
    len = strlen(buf);
    snprintf(nbuf, n, "%s", buf);
    return len;
}

int crio_list2str(CrioList *list, char *buf, size_t n)
{
    CrioList *h = list;
    char *bufp = buf;
    int len;

    if (!list) {
        snprintf(buf, n, "%s", "()");
        return 3;
    }
    snprintf(buf, n, "%s", "(");
    len = 1;
    bufp += len;
    while (len < n - 1) {
        len = crio_node2str(CRIO_CAR(h), bufp, n - len);
        bufp += len;
        h = CRIO_CDR(h);
        if (!CRIO_IS_NIL(h)) {
            snprintf(bufp, n - len, "%s", ", ");
            bufp += 2;
        }
        else break;
    }
    snprintf(bufp, n, "%s", ")");
    return strlen(buf);
}

CrioNode
crio_mknode_int(int v)
{
    CrioNode node = xmalloc(sizeof(struct _crio_node));
    if (!node) return NULL;
    CRIO_TYPE(node) = CRIO_INT_T;
    CRIO_VALUE(node) = v;
    return node;
}

CrioNode
crio_mknode_fun(int (*fun)(CrioList *, struct crio_stream *))
{
    CrioNode node = xmalloc(sizeof(struct _crio_node));
    if (!node) return NULL;
    CRIO_TYPE(node) = CRIO_FUN_T;
    CRIO_FUN(node) = fun;
    return node;
}

CrioNode
crio_mknode_filter(struct crio_filter *filter)
{
    CrioNode node = xmalloc(sizeof(struct _crio_node));
    if (!node) return NULL;
    CRIO_TYPE(node) = CRIO_FILTER_T;
    CRIO_FILTER(node) = filter;
    return node;
}

CrioNode
crio_mknode_list(CrioList *list)
{
    CrioNode node = xmalloc(sizeof(struct _crio_node));
    if (!node) return NULL;
    CRIO_TYPE(node) = CRIO_LIST_T;
    CRIO_LIST(node) = list;
    return node;
}

CrioList *
crio_cons(CrioNode node, CrioList *list)
{
    CrioList *head = list;
    if (!list) head = LIST_NIL;
    CrioList *e = xmalloc(sizeof(CrioList));
    if (!e) return NULL;
    e->node = (CrioNode )node;
    e->next = head;
    return e;
}

int crio_list_length(CrioList *list)
{
    int len = 0;
    CrioList *h = list;
    if (list) {
        while (!CRIO_IS_NIL(h)) {
            len++;
            h = CRIO_CDR(h);
        }
    }
    return len;
}

static void free_nodes_in_list(CrioList *list)
{
    CrioList *h = list;
    CrioNode n;
    if (list) {
        while (!CRIO_IS_NIL(h)) {
            n = CRIO_CAR(h);
            if (n && (n != &FUN_NODE_OR) && (n != &FUN_NODE_AND)) {
                free(CRIO_CAR(h));
                CRIO_CAR(h) = NULL;
            }
            h = CRIO_CDR(h);
        }
    }
}

void crio_list_free(CrioList *list, int keep_nodes)
{
    CrioList *h = list, *tmp;
    if (!keep_nodes) free_nodes_in_list(list);
    if (list) {
        while (!CRIO_IS_NIL(h)) {
            tmp = CRIO_CDR(h);
            free(h);
            h = tmp;
        }
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


CrioNode
crio_eval_filter(CrioNode e,
                 struct crio_stream *stream)
{
    struct crio_filter *cf = CRIO_FILTER(e);
    int res = cf->filter(stream, cf->filter_ctx);
    CrioNode node = xmalloc(sizeof(struct _crio_node));
    if (!node) return NULL;
    CRIO_TYPE(node) = CRIO_INT_T;
    CRIO_VALUE(node) = res;
    return node;
}

CrioNode
crio_eval_fun(CrioNode e, CrioList *args, struct crio_stream *stream)
{
    int res = CRIO_FUN(e)(args, stream);
    CrioNode node = xmalloc(sizeof(struct _crio_node));
    if (!node) return NULL;
    CRIO_TYPE(node) = CRIO_INT_T;
    CRIO_VALUE(node) = res;
    return node;
}

static int crio_fun_and(CrioList *args, struct crio_stream *stream)
{
    CrioList *a = args;
    CrioNode n;
    int ans = 1;
    while (!CRIO_IS_NIL(a)) {
        n = CRIO_CAR(a);
        if (!IS_CRIO_INT_T(n))
            n = _crio_eval(crio_cons(n, NULL), stream);
        if (CRIO_ERR == CRIO_VALUE(n)) return CRIO_VALUE(n);
        ans = ans && CRIO_VALUE(n);
        if (!ans) break;
        a = CRIO_CDR(a);
    }
    return ans;
}

static int crio_fun_or(CrioList *args, struct crio_stream *stream)
{
    CrioList *a = args;
    CrioNode n;
    int ans = 0;
    while (!CRIO_IS_NIL(a)) {
        n = CRIO_CAR(a);
        if (!IS_CRIO_INT_T(n))
            n = _crio_eval(crio_cons(n, NULL), stream);
        if (CRIO_ERR == CRIO_VALUE(n)) return CRIO_VALUE(n);
        ans = ans || CRIO_VALUE(n);
        if (ans) break;
        a = CRIO_CDR(a);
    }
    return ans;
}

static int crio_fun_not(CrioList *args, struct crio_stream *stream)
{
    CrioNode n;
    int ans = CRIO_ERR;
    if (!CRIO_IS_NIL(args)) {
        n = CRIO_CAR(args);
        if (!IS_CRIO_INT_T(n))
            n = _crio_eval(crio_cons(n, NULL), stream);
        if (CRIO_ERR == CRIO_VALUE(n)) return CRIO_VALUE(n);
        ans = CRIO_VALUE(n) ? 0 : 1;
    }
    return ans;
}

CrioNode  crio_mknode_fun_and()
{
    return &FUN_NODE_AND;
}

CrioNode  crio_mknode_fun_or()
{
    return &FUN_NODE_OR;
}

CrioNode crio_mknode_fun_not()
{
    return &FUN_NODE_NOT;
}

CrioNode
_crio_eval(CrioList *list, struct crio_stream *stream)
{
    CrioList *e = list;
    CrioNode v = CRIO_CAR(list);

    switch (CRIO_TYPE(v)) {
    case CRIO_INT_T:
        return v;
    case CRIO_FUN_T:
        /* crio funs are expected to evaluate their arguments if needed.
           This allows for short-circuit logic funs like & and |.
         */
        return crio_eval_fun(v, CRIO_CDR(e), stream);
    case CRIO_FILTER_T:
        return crio_eval_filter(v, stream);
    case CRIO_LIST_T:
        return _crio_eval(CRIO_LIST(v), stream);
    }

    return NULL;                /* should not happen */
}
