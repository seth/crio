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

static int demo_read(struct crio_stream *stream)
{
    FILE *file = (FILE *)stream->file;
    char *buf = (char *)stream->ctx;
    int c = fscanf(file, "%s", buf);
    if (c == 1) return CRIO_OK;
    if (c == -1) return CRIO_EOF;
    return CRIO_ERR;
}

static int demo_read_noop(struct crio_stream *stream)
{
    return CRIO_OK;
}

static int demo_read_int(struct crio_stream *stream)
{
    int res = CRIO_EOF;
    struct demo_int_ctx *ctx = (struct demo_int_ctx *)stream->ctx;
    char *data[] = {"123", "foo", "2", "3", "bar"};
    if (ctx->file_index < 5) {
        char *endptr;
        ctx->value = (int)strtol(data[ctx->file_index], &endptr, 10);
        if (endptr == data[ctx->file_index]) {
            crio_set_errmsg(stream, "demo_read_int: can't parse int from '%s'",
                            data[ctx->file_index]);
            res = CRIO_ERR;
        } else {
            res = CRIO_OK;
        }
        ctx->file_index++;
    }
    return res;
}

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

static int filter_with_error(struct crio_stream *stream, void *fctx)
{
    struct demo_int_ctx *ctx = (struct demo_int_ctx *)stream->ctx;
    crio_set_errmsg(stream, "filter_with_error failed with: %d", ctx->value);
    return CRIO_ERR;
}

static int op_fun(CrioList *list) {
    return 1;
}

void Test_node_making(CuTest *tc)
{
    CrioList *list = NULL;

    CrioNode *n = crio_mknode_int(5);
    list = crio_cons(n, list);

    n = crio_mknode_fun(op_fun);
    list = crio_cons(n, list);

    struct crio_filter *cf = crio_filter_make("t1", alpha_filter, NULL, NULL);
    n = crio_mknode_filter(cf);
    list = crio_cons(n, list);

    crio_print_list(list);
    CrioList *rlist = crio_list_reverse(list);
    crio_list_free(list);
    crio_print_list(rlist);
    crio_list_free(rlist);
}

void Test_ast_identity_eval(CuTest *tc)
{
    CrioNode *n = crio_mknode_int(4), *ans;
    CrioList *list = crio_cons(n , NULL);
    ans = _crio_eval(list, NULL);
    CuAssertIntEquals(tc, 4, CRIO_VALUE(ans));
    crio_list_free(list);
    free(n);
}

void Test_ast_fun_AND_eval_simple(CuTest *tc)
{
    CrioList *list;
    CrioNode
        *ans, *n1 = crio_mknode_int(1),
        *n0 = crio_mknode_int(0),
        *fun_and = crio_mknode_fun_and();

    /* test case:  (and 1 1) */
    list = crio_cons(fun_and,
                     crio_cons(n1, crio_cons(n1 , NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 1, CRIO_VALUE(ans));
    crio_list_free(list);

    /* test case:  (and 0 1) */
    list = crio_cons(fun_and,
                     crio_cons(n0, crio_cons(n1 , NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 0, CRIO_VALUE(ans));
    crio_list_free(list);

    /* test case:  (and 1 0) */
    list = crio_cons(fun_and,
                     crio_cons(n1, crio_cons(n0 , NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 0, CRIO_VALUE(ans));
    crio_list_free(list);

    /* test case:  (and 0 0) */
    list = crio_cons(fun_and,
                     crio_cons(n0, crio_cons(n0 , NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 0, CRIO_VALUE(ans));
    crio_list_free(list);
}

void Test_ast_fun_OR_eval_simple(CuTest *tc)
{
    CrioList *list;
    CrioNode
        *ans, *n1 = crio_mknode_int(1),
        *n0 = crio_mknode_int(0),
        *fun_or = crio_mknode_fun_or();

    /* test case:  (or 1 1) */
    list = crio_cons(fun_or,
                     crio_cons(n1, crio_cons(n1 , NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 1, CRIO_VALUE(ans));
    crio_list_free(list);

    /* test case:  (or 0 1) */
    list = crio_cons(fun_or,
                     crio_cons(n0, crio_cons(n1 , NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 1, CRIO_VALUE(ans));
    crio_list_free(list);

    /* test case:  (or 1 0) */
    list = crio_cons(fun_or,
                     crio_cons(n1, crio_cons(n0 , NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 1, CRIO_VALUE(ans));
    crio_list_free(list);

    /* test case:  (or 0 0) */
    list = crio_cons(fun_or,
                     crio_cons(n0, crio_cons(n0 , NULL)));
    ans = _crio_eval(list, NULL);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 0, CRIO_VALUE(ans));
    crio_list_free(list);
}

void Test_ast_filter_eval_simple(CuTest *tc)
{
    CrioList *list;
    char ctx[256];
    struct crio_filter *cf = crio_filter_make("alpha", alpha_filter,
                                              NULL, NULL);
    CrioNode *ans,
        *filt_node = crio_mknode_filter(cf);

    struct crio_stream *stream = crio_stream_make(NULL, NULL,
                                                  "dri", &ctx);

    list = crio_cons(filt_node, NULL);
    strcpy(ctx, "no digits here");
    ans = _crio_eval(list, stream);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 1, CRIO_VALUE(ans));

    strcpy(ctx, "some 123 digits here");
    ans = _crio_eval(list, stream);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 0, CRIO_VALUE(ans));
}

void Test_ast_filter_eval_compound1(CuTest *tc)
{
    CrioList *list;
    char ctx[256];
    struct crio_filter *cfA = crio_filter_make("alpha", alpha_filter,
                                              NULL, NULL),
        *cfK = crio_filter_make("hask", has_k_filter, NULL, NULL);
    CrioNode *ans,
        *filtA_node = crio_mknode_filter(cfA),
        *filtK_node = crio_mknode_filter(cfK),
        *fun_or = crio_mknode_fun_or(),
        *fun_and = crio_mknode_fun_and(),
        *n1 = crio_mknode_int(1);

    struct crio_stream *stream = crio_stream_make(NULL, NULL,
                                                  "dri", &ctx);
    /* (or has_k no_digits) */
    list = crio_cons(fun_or,
                     crio_cons(filtK_node,
                               crio_cons(filtA_node, NULL)));
    strcpy(ctx, "no digits here");
    ans = _crio_eval(list, stream);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 1, CRIO_VALUE(ans));
    crio_list_free(list);

    /* (and has_k no_digits) */
    list = crio_cons(fun_and,
                     crio_cons(filtK_node,
                               crio_cons(filtA_node, NULL)));
    ans = _crio_eval(list, stream);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 0, CRIO_VALUE(ans));
    crio_list_free(list);

    /* (or 1 (and has_k no_digits)) */
    list = crio_cons(fun_and,
                     crio_cons(filtK_node,
                               crio_cons(filtA_node, NULL)));
    list = crio_cons(fun_or, crio_cons(n1, list));
    ans = _crio_eval(list, stream);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 1, CRIO_VALUE(ans));
    crio_list_free(list);
}




