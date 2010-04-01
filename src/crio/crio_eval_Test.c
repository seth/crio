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

static int op_fun(CrioList *list) {
    return 1;
}

void Test_node_making(CuTest *tc)
{
    CrioList *list = NULL;
    struct _crio_mpool *pool = crio_mpool_make0(sizeof(char) * 1024);
    crio_set_global_mem_pool(pool);
    CrioNode n = crio_mknode_int(5);
    list = crio_cons(n, list);

    n = crio_mknode_fun(op_fun);
    list = crio_cons(n, list);

    struct crio_filter *cf = crio_filter_make("t1", alpha_filter, NULL, NULL);
    n = crio_mknode_filter(cf);
    list = crio_cons(n, list);

    crio_print_list(list);
    CrioList *rlist = crio_list_reverse(list);
    /* crio_list_free(list, 1); */
    crio_print_list(rlist);
    /* crio_list_free(rlist, 0); */

    crio_mpool_free(pool);
    crio_set_global_mem_pool(NULL);
}

void Test_ast_identity_eval(CuTest *tc)
{
    CrioNode n = crio_mknode_int(4), ans;
    CrioList *list = crio_cons(n , NULL);
    ans = _crio_eval(list, NULL);
    CuAssertIntEquals(tc, 4, CRIO_VALUE(ans));
    crio_list_free(list, 0);
}

void Test_ast_fun_AND_error_propagation(CuTest *tc)
{
    CrioList *list;
    CrioNode
        ans, nErr = crio_mknode_int(CRIO_ERR),
        n1 = crio_mknode_int(1),
        n0 = crio_mknode_int(0),
        fun_and = crio_mknode_fun_and();

    list = crio_cons(fun_and, crio_cons(nErr, NULL));
    ans = _crio_eval(list, NULL);
    CuAssertIntEquals(tc, CRIO_ERR, CRIO_VALUE(ans));
    crio_list_free(list, 1);

    list = crio_cons(fun_and, crio_cons(n1, crio_cons(nErr, NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertIntEquals(tc, CRIO_ERR, CRIO_VALUE(ans));
    crio_list_free(list, 1);

    list = crio_cons(fun_and, crio_cons(n0, crio_cons(nErr, NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertIntEquals(tc, CRIO_ERR, CRIO_VALUE(ans));
    crio_list_free(list, 1);

    /* FIXME: if we would short-circuit properly, this would return
       false instead of error.  Also evaluation order is backwards.
     */
    list = crio_cons(fun_and,
                     crio_cons(n0,
                               crio_cons(n1, crio_cons(nErr, NULL))));
    ans = _crio_eval(list, NULL);
    CuAssertIntEquals(tc, CRIO_ERR, CRIO_VALUE(ans));
    crio_list_free(list, 0);
}

void Test_ast_fun_OR_error_propagation(CuTest *tc)
{
    CrioList *list;
    CrioNode
        ans, nErr = crio_mknode_int(CRIO_ERR),
        n1 = crio_mknode_int(1),
        n0 = crio_mknode_int(0),
        fun_or = crio_mknode_fun_or();

    list = crio_cons(fun_or, crio_cons(nErr, NULL));
    ans = _crio_eval(list, NULL);
    CuAssertIntEquals(tc, CRIO_ERR, CRIO_VALUE(ans));
    crio_list_free(list, 1);

    list = crio_cons(fun_or, crio_cons(n1, crio_cons(nErr, NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertIntEquals(tc, CRIO_ERR, CRIO_VALUE(ans));
    crio_list_free(list, 1);

    list = crio_cons(fun_or, crio_cons(n0, crio_cons(nErr, NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertIntEquals(tc, CRIO_ERR, CRIO_VALUE(ans));
    crio_list_free(list, 1);

    /* FIXME: if we would short-circuit properly, this would return
       false instead of error.  Also evaluation order is backwards.
     */
    list = crio_cons(fun_or,
                     crio_cons(n0,
                               crio_cons(n1, crio_cons(nErr, NULL))));
    ans = _crio_eval(list, NULL);
    CuAssertIntEquals(tc, CRIO_ERR, CRIO_VALUE(ans));
    crio_list_free(list, 0);
}

void Test_ast_fun_AND_eval_simple(CuTest *tc)
{
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
    crio_list_free(list, 1);

    /* test case:  (and 0 1) */
    list = crio_cons(fun_and,
                     crio_cons(n0, crio_cons(n1 , NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 0, CRIO_VALUE(ans));
    crio_list_free(list, 1);

    /* test case:  (and 1 0) */
    list = crio_cons(fun_and,
                     crio_cons(n1, crio_cons(n0 , NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 0, CRIO_VALUE(ans));
    crio_list_free(list, 1);

    /* test case:  (and 0 0) */
    list = crio_cons(fun_and,
                     crio_cons(n0, crio_cons(n0 , NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 0, CRIO_VALUE(ans));
    crio_list_free(list, 1);
    free(ans);
    free(n1);
    free(n0);
    free(fun_and);
}

void Test_ast_fun_OR_eval_simple(CuTest *tc)
{
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
    crio_list_free(list, 1);

    /* test case:  (or 0 1) */
    list = crio_cons(fun_or,
                     crio_cons(n0, crio_cons(n1 , NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 1, CRIO_VALUE(ans));
    crio_list_free(list, 1);

    /* test case:  (or 1 0) */
    list = crio_cons(fun_or,
                     crio_cons(n1, crio_cons(n0 , NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 1, CRIO_VALUE(ans));
    crio_list_free(list, 1);

    /* test case:  (or 0 0) */
    list = crio_cons(fun_or,
                     crio_cons(n0, crio_cons(n0 , NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 0, CRIO_VALUE(ans));
    crio_list_free(list, 1);
    free(ans);
    free(n1);
    free(n0);
    free(fun_or);
}

void Test_ast_filter_eval_simple(CuTest *tc)
{
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
    crio_list_free(list, 0);
}

void Test_ast_filter_eval_compound1(CuTest *tc)
{
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
    crio_list_free(list, 1);

    /* (and has_k no_digits) */
    list = crio_cons(fun_and,
                     crio_cons(filtK_node,
                               crio_cons(filtA_node, NULL)));
    stream->filter = crio_mknode_list(list);
    ans = _crio_eval(list, stream);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 0, CRIO_VALUE(ans));
    crio_list_free(list, 1);

    /* (or 1 (and has_k no_digits)) */
    list = crio_cons(fun_and,
                     crio_cons(filtK_node,
                               crio_cons(filtA_node, NULL)));
    list = crio_cons(fun_or, crio_cons(n1, list));
    stream->filter = crio_mknode_list(list);
    ans = _crio_eval(list, stream);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 1, CRIO_VALUE(ans));
    crio_list_free(list, 1);

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
    crio_list_free(list, 0);
}




