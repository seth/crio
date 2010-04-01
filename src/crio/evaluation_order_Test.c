#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "CuTest.h"
#include "crio_eval.h"

struct my_ctx {
    char *buf;
    char *cur;
    int size;
};

struct marking_ctx {
    int pass;
    char *name;
};

static int marking_filter(struct crio_stream *stream, void *fctx)
{
    struct my_ctx *mc = (struct my_ctx *)stream->ctx;
    struct marking_ctx *fc = (struct marking_ctx *)fctx;
    strncat(mc->buf, fc->name, mc->size - 1);
    return fc->pass ? CRIO_FILT_PASS : CRIO_FILT_FAIL;
}

void Test_evaluation_order_2_arg_and_or(CuTest *tc)
{
    CrioList *expr;
    struct crio_filter *cf_a1, *cf_b0;
    struct marking_ctx ftx_a_1;
    struct marking_ctx ftx_b_0;
    struct my_ctx ctx;
    struct crio_stream *stream;
    CrioNode ans;
    struct _crio_mpool *pool = crio_mpool_make(256);
    size_t mark;

    crio_set_global_mem_pool(pool);

    ctx.buf = malloc(sizeof(char) * 256);
    memset(ctx.buf, 0, sizeof(char) * 256);
    ctx.size = 256;
    ctx.cur = NULL;

    ftx_a_1.name = "a";
    ftx_a_1.pass = 1;

    ftx_b_0.name = "b";
    ftx_b_0.pass = 0;
    
    cf_a1 = crio_filter_make("a1", marking_filter, &ftx_a_1, NULL);
    cf_b0 = crio_filter_make("b0", marking_filter, &ftx_b_0, NULL);

    stream = crio_stream_make(NULL, NULL, "eorder", &ctx, NULL);

    /* AND */

    /* a1 & b0: both evaluated */
    mark = crio_mpool_mark(pool);
    expr = crio_cons(crio_mknode_fun_and(),
                     crio_cons(crio_mknode_filter(cf_a1),
                               crio_cons(crio_mknode_filter(cf_b0),
                                         NULL)));
    ans = _crio_eval(expr, stream);
    CuAssertTrue(tc, IS_CRIO_INT_T(ans));
    CuAssertIntEquals(tc, 0, CRIO_VALUE(ans));
    CuAssertStrEquals(tc, "ab", ctx.buf);
    crio_mpool_drain_to_mark(pool, mark);

    /* b0 & a1: only b0 is evaluated */
    memset(ctx.buf, 0, sizeof(char) * 256);
    mark = crio_mpool_mark(pool);
    expr = crio_cons(crio_mknode_fun_and(),
                     crio_cons(crio_mknode_filter(cf_b0),
                               crio_cons(crio_mknode_filter(cf_a1),
                                         NULL)));
    ans = _crio_eval(expr, stream);
    CuAssertIntEquals(tc, 0, CRIO_VALUE(ans));
    CuAssertStrEquals(tc, "b", ctx.buf);
    crio_mpool_drain_to_mark(pool, mark);

    /* a1 & a1: both evaluated */
    memset(ctx.buf, 0, sizeof(char) * 256);
    mark = crio_mpool_mark(pool);
    expr = crio_cons(crio_mknode_fun_and(),
                     crio_cons(crio_mknode_filter(cf_a1),
                               crio_cons(crio_mknode_filter(cf_a1),
                                         NULL)));
    ans = _crio_eval(expr, stream);
    CuAssertIntEquals(tc, 1, CRIO_VALUE(ans));
    CuAssertStrEquals(tc, "aa", ctx.buf);
    crio_mpool_drain_to_mark(pool, mark);

    /* b0 & b0: b0 evaluated once */
    memset(ctx.buf, 0, sizeof(char) * 256);
    mark = crio_mpool_mark(pool);
    expr = crio_cons(crio_mknode_fun_and(),
                     crio_cons(crio_mknode_filter(cf_b0),
                               crio_cons(crio_mknode_filter(cf_b0),
                                         NULL)));
    ans = _crio_eval(expr, stream);
    CuAssertIntEquals(tc, 0, CRIO_VALUE(ans));
    CuAssertStrEquals(tc, "b", ctx.buf);
    crio_mpool_drain_to_mark(pool, mark);

    /* OR */

    /* b0 | b0: b0 evaluated twice */
    memset(ctx.buf, 0, sizeof(char) * 256);
    mark = crio_mpool_mark(pool);
    expr = crio_cons(crio_mknode_fun_or(),
                     crio_cons(crio_mknode_filter(cf_b0),
                               crio_cons(crio_mknode_filter(cf_b0),
                                         NULL)));
    ans = _crio_eval(expr, stream);
    CuAssertIntEquals(tc, 0, CRIO_VALUE(ans));
    CuAssertStrEquals(tc, "bb", ctx.buf);
    crio_mpool_drain_to_mark(pool, mark);

    /* b0 | a1: both evaluated  */
    memset(ctx.buf, 0, sizeof(char) * 256);
    mark = crio_mpool_mark(pool);
    expr = crio_cons(crio_mknode_fun_or(),
                     crio_cons(crio_mknode_filter(cf_b0),
                               crio_cons(crio_mknode_filter(cf_a1),
                                         NULL)));
    ans = _crio_eval(expr, stream);
    CuAssertIntEquals(tc, 1, CRIO_VALUE(ans));
    CuAssertStrEquals(tc, "ba", ctx.buf);
    crio_mpool_drain_to_mark(pool, mark);

    /* a1 | b0: a1 evaluated  */
    memset(ctx.buf, 0, sizeof(char) * 256);
    mark = crio_mpool_mark(pool);
    expr = crio_cons(crio_mknode_fun_or(),
                     crio_cons(crio_mknode_filter(cf_a1),
                               crio_cons(crio_mknode_filter(cf_b0),
                                         NULL)));
    ans = _crio_eval(expr, stream);
    CuAssertIntEquals(tc, 1, CRIO_VALUE(ans));
    CuAssertStrEquals(tc, "a", ctx.buf);
    crio_mpool_drain_to_mark(pool, mark);


    /* clean up */
    crio_set_global_mem_pool(NULL);
    crio_mpool_free(pool);
}
