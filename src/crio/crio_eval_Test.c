#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "CuTest.h"
#include "crio_eval.h"

struct demo_int_ctx {
    int value;
    int file_index;
};

static int alpha_filter(struct crio_stream *stream, void *fctx)
{
    char *buf = (char *)stream->ctx;
    for ( ; buf[0]; buf++) {
        if (isdigit(buf[0])) return CRIO_FILT_FAIL;
    }
    return CRIO_FILT_PASS;
}

static int has_k_filter(struct crio_stream *stream, void *fctx)
{

    char *buf = (char *)stream->ctx;
    int res = CRIO_FILT_FAIL;
    for ( ; buf[0]; buf++) {
        if (buf[0] == 'k') res = CRIO_FILT_PASS;
    }
    return res;
}

static int op_fun(CrioList *list, struct crio_stream *stream) {
    return 1;
}

static struct _crio_mpool *CUTEST_POOL;

void _setup_mem_pool()
{
    CUTEST_POOL = crio_mpool_make(256);
    crio_set_global_mem_pool(CUTEST_POOL);
}

void _teardown_mem_pool()
{
    crio_mpool_free(CUTEST_POOL);
    crio_set_global_mem_pool(NULL);
}

void Test_node_making(CuTest *tc)
{
    char buf[256];
    CrioList *list = NULL, *rlist;
    struct crio_filter *cf;
    _setup_mem_pool();

    CrioNode n = crio_mknode_int(5);
    crio_node2str(n, buf, 256);
    CuAssertStrEquals(tc, "[INT (5)]", buf);

    list = crio_cons(n, list);
    list = crio_cons(crio_mknode_fun_and(), list);

    cf = crio_filter_make("t1", alpha_filter, NULL, NULL);
    n = crio_mknode_filter(cf);
    list = crio_cons(n, list);

    crio_list2str(list, buf, 256);
    CuAssertStrEquals(tc, 
                      "([FILTER (t1)], [FUN (&)], [INT (5)])",
                      buf);

    rlist = crio_list_reverse(list);
    crio_list2str(rlist, buf, 256);
    CuAssertStrEquals(tc, 
                      "([INT (5)], [FUN (&)], [FILTER (t1)])",
                      buf);

    _teardown_mem_pool();
}


void Test_ast_identity_eval(CuTest *tc)
{
    CrioList *list;
    CrioNode n, ans;
    _setup_mem_pool();
    n = crio_mknode_int(4);
    list = crio_cons(n , NULL);
    ans = _crio_eval(list, NULL);
    CuAssertIntEquals(tc, 4, CRIO_VALUE(ans));
    _teardown_mem_pool();
}

void Test_ast_fun_AND_error_propagation(CuTest *tc)
{
    _setup_mem_pool();
    CrioList *list;
    CrioNode
        ans, nErr = crio_mknode_int(CRIO_ERR),
        n1 = crio_mknode_int(1),
        n0 = crio_mknode_int(0),
        fun_and = crio_mknode_fun_and();
    list = crio_cons(fun_and, crio_cons(nErr, NULL));
    ans = _crio_eval(list, NULL);
    CuAssertIntEquals(tc, CRIO_ERR, CRIO_VALUE(ans));

    list = crio_cons(fun_and, crio_cons(n1, crio_cons(nErr, NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertIntEquals(tc, CRIO_ERR, CRIO_VALUE(ans));

    /* short-circuit prevents error */
    list = crio_cons(fun_and, crio_cons(n0, crio_cons(nErr, NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertIntEquals(tc, 0, CRIO_VALUE(ans));

    list = crio_cons(fun_and, crio_cons(nErr, crio_cons(n0, NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertIntEquals(tc, CRIO_ERR, CRIO_VALUE(ans));

    list = crio_cons(fun_and,
                     crio_cons(n0,
                               crio_cons(n1, crio_cons(nErr, NULL))));
    ans = _crio_eval(list, NULL);
    CuAssertIntEquals(tc, 0, CRIO_VALUE(ans));

    _teardown_mem_pool();
}

void Test_ast_fun_OR_error_propagation(CuTest *tc)
{
    _setup_mem_pool();
    CrioList *list;
    CrioNode
        ans, nErr = crio_mknode_int(CRIO_ERR),
        n1 = crio_mknode_int(1),
        n0 = crio_mknode_int(0),
        fun_or = crio_mknode_fun_or();

    list = crio_cons(fun_or, crio_cons(nErr, NULL));
    ans = _crio_eval(list, NULL);
    CuAssertIntEquals(tc, CRIO_ERR, CRIO_VALUE(ans));

    /* short-circuit prevents error */
    list = crio_cons(fun_or, crio_cons(n1, crio_cons(nErr, NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertIntEquals(tc, 1, CRIO_VALUE(ans));

    list = crio_cons(fun_or, crio_cons(n0, crio_cons(nErr, NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertIntEquals(tc, CRIO_ERR, CRIO_VALUE(ans));

    list = crio_cons(fun_or,
                     crio_cons(n0,
                               crio_cons(n1, crio_cons(nErr, NULL))));
    ans = _crio_eval(list, NULL);
    CuAssertIntEquals(tc, 1, CRIO_VALUE(ans));
    _teardown_mem_pool();
}

void Test_ast_fun_AND_eval_simple(CuTest *tc)
{
    _setup_mem_pool();
    CrioList *list;
    CrioNode
        ans, n1 = crio_mknode_int(1),
        n0 = crio_mknode_int(0),
        fun_and = crio_mknode_fun_and();

    /* test case:  (and 1 1) */
    list = crio_cons(fun_and,
                     crio_cons(n1, crio_cons(n1 , NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 1, CRIO_VALUE(ans));

    /* test case:  (and 0 1) */
    list = crio_cons(fun_and,
                     crio_cons(n0, crio_cons(n1 , NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 0, CRIO_VALUE(ans));

    /* test case:  (and 1 0) */
    list = crio_cons(fun_and,
                     crio_cons(n1, crio_cons(n0 , NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 0, CRIO_VALUE(ans));

    /* test case:  (and 0 0) */
    list = crio_cons(fun_and,
                     crio_cons(n0, crio_cons(n0 , NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 0, CRIO_VALUE(ans));
    _teardown_mem_pool();
}

void Test_ast_fun_OR_eval_simple(CuTest *tc)
{
    _setup_mem_pool();
    CrioList *list;
    CrioNode
        ans, n1 = crio_mknode_int(1),
        n0 = crio_mknode_int(0),
        fun_or = crio_mknode_fun_or();

    /* test case:  (or 1 1) */
    list = crio_cons(fun_or,
                     crio_cons(n1, crio_cons(n1 , NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 1, CRIO_VALUE(ans));

    /* test case:  (or 0 1) */
    list = crio_cons(fun_or,
                     crio_cons(n0, crio_cons(n1 , NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 1, CRIO_VALUE(ans));

    /* test case:  (or 1 0) */
    list = crio_cons(fun_or,
                     crio_cons(n1, crio_cons(n0 , NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 1, CRIO_VALUE(ans));

    /* test case:  (or 0 0) */
    list = crio_cons(fun_or,
                     crio_cons(n0, crio_cons(n0 , NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 0, CRIO_VALUE(ans));
    _teardown_mem_pool();
}

void Test_ast_filter_eval_simple(CuTest *tc)
{
    _setup_mem_pool();
    CrioList *list;
    char ctx[256];
    struct crio_filter *cf = crio_filter_make("alpha", alpha_filter,
                                              NULL, NULL);
    CrioNode ans,
        filt_node = crio_mknode_filter(cf);

    struct crio_stream *stream = crio_stream_make(NULL, NULL,
                                                  "dri", &ctx, NULL);

    list = crio_cons(filt_node, NULL);
    strcpy(ctx, "no digits here");
    stream->filter = crio_mknode_list(list);
    ans = _crio_eval(list, stream);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 1, CRIO_VALUE(ans));

    strcpy(ctx, "some 123 digits here");
    ans = _crio_eval(list, stream);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 0, CRIO_VALUE(ans));
    _teardown_mem_pool();
}

void Test_ast_filter_eval_compound1(CuTest *tc)
{
    _setup_mem_pool();
    CrioList *list;
    char ctx[256];
    struct crio_filter *cfA = crio_filter_make("alpha", alpha_filter,
                                              NULL, NULL),
        *cfK = crio_filter_make("hask", has_k_filter, NULL, NULL);
    CrioNode ans,
        filtA_node = crio_mknode_filter(cfA),
        filtK_node = crio_mknode_filter(cfK),
        fun_or = crio_mknode_fun_or(),
        fun_and = crio_mknode_fun_and(),
        n1 = crio_mknode_int(1);

    struct crio_stream *stream = crio_stream_make(NULL, NULL,
                                                  "dri", &ctx,
                                                  NULL);

    /* (or has_k no_digits) */
    list = crio_cons(fun_or,
                     crio_cons(filtK_node,
                               crio_cons(filtA_node, NULL)));
    /* FIXME: do we want this sort of function? */
    /* crio_stream_set_filter(stream, crio_mknode_list(list)); */
    stream->filter = crio_mknode_list(list);
    strcpy(ctx, "no digits here");
    ans = _crio_eval(list, stream);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 1, CRIO_VALUE(ans));

    /* (and has_k no_digits) */
    list = crio_cons(fun_and,
                     crio_cons(filtK_node,
                               crio_cons(filtA_node, NULL)));
    stream->filter = crio_mknode_list(list);
    ans = _crio_eval(list, stream);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 0, CRIO_VALUE(ans));

    /* (or 1 (and has_k no_digits)) */
    list = crio_cons(fun_and,
                     crio_cons(filtK_node,
                               crio_cons(filtA_node, NULL)));
    list = crio_cons(fun_or, crio_cons(n1, list));
    stream->filter = crio_mknode_list(list);
    ans = _crio_eval(list, stream);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 1, CRIO_VALUE(ans));

    /* (or (and has_k no_digits) 1) */
    /* this tests an AST where CAR(s) is itself a list */
    list = crio_cons(fun_and,
                     crio_cons(filtK_node,
                               crio_cons(filtA_node, NULL)));
    list = crio_cons(fun_or, 
                     crio_cons(crio_mknode_list(list),
                               crio_cons(n1, NULL)));
    stream->filter = crio_mknode_list(list);
    ans = _crio_eval(list, stream);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 1, CRIO_VALUE(ans));
    _teardown_mem_pool();
}

void Test_crio_not_fun(CuTest *tc)
{
    CrioList *expr;
    CrioNode ans;
    struct _crio_mpool *pool = crio_mpool_make(256);
    size_t mark;
    crio_set_global_mem_pool(pool);

    /* !1 => 0 */
    mark = crio_mpool_mark(pool);
    expr = crio_cons(crio_mknode_fun_not(),
                     crio_cons(crio_mknode_int(1), NULL));
    ans = _crio_eval(expr, NULL);
    CuAssertIntEquals(tc, 0, CRIO_VALUE(ans));
    crio_mpool_drain_to_mark(pool, mark);

    /* !0 => 1 */
    mark = crio_mpool_mark(pool);
    expr = crio_cons(crio_mknode_fun_not(),
                     crio_cons(crio_mknode_int(0), NULL));
    ans = _crio_eval(expr, NULL);
    CuAssertIntEquals(tc, 1, CRIO_VALUE(ans));
    crio_mpool_drain_to_mark(pool, mark);

    /* TODO: test not for evaluation of simple arg */
    /* TODO: test !(1 & (1 | 0)) */
    mark = crio_mpool_mark(pool);
    expr = crio_cons(crio_mknode_fun_or(),
                     crio_cons(crio_mknode_int(0),
                               crio_cons(crio_mknode_int(1), NULL)));
    expr = crio_cons(crio_mknode_fun_not(),
                     crio_cons(crio_mknode_fun_and(),
                               crio_cons(crio_mknode_int(1), expr)));
    ans = _crio_eval(expr, NULL);
    CuAssertIntEquals(tc, 0, CRIO_VALUE(ans));
    crio_mpool_drain_to_mark(pool, mark);

    /* clean up */
    crio_set_global_mem_pool(NULL);
    crio_mpool_free(pool);
}
