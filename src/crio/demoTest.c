#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "CuTest.h"
#include "crio.h"

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

void Test_demo_crio_err(CuTest *tc)
{
    int res;
    struct demo_int_ctx ctx;
    ctx.value = 0;
    ctx.file_index = 0;

    struct crio_stream *stream = crio_stream_make(demo_read_int, NULL,
                                                  "dri", &ctx);
    res = crio_next(stream);
    CuAssertIntEquals(tc, CRIO_OK, res);
    CuAssertIntEquals(tc, 123, ctx.value);

    res = crio_next(stream);
    CuAssertIntEquals(tc, CRIO_ERR, res);
    CuAssertStrEquals(tc, "demo_read_int: can't parse int from 'foo'"
                      " [file: dri, record: 2]",
                      crio_errmsg(stream));
    /* make sure this is idempotent */
    CuAssertStrEquals(tc, "demo_read_int: can't parse int from 'foo'"
                      " [file: dri, record: 2]",
                      crio_errmsg(stream));

    res = crio_next(stream);
    res = crio_next(stream);
    res = crio_next(stream);
    CuAssertStrEquals(tc, "demo_read_int: can't parse int from 'bar'"
                      " [file: dri, record: 5]",
                      crio_errmsg(stream));
    CuAssertIntEquals(tc, CRIO_EOF, crio_next(stream));
    CuAssertIntEquals(tc, 5, stream->nread);
    crio_stream_free(stream);
}

void Test_filter_has_error(CuTest *tc)
{
    struct demo_int_ctx ctx;
    ctx.value = 4;
    struct crio_stream *stream = crio_stream_make(demo_read_noop, NULL, "noop", 
                                                  &ctx);
    crio_add_filter(stream, "ferr", filter_with_error, NULL);
    int res = crio_next(stream);
    CuAssertIntEquals(tc, CRIO_ERR, res);
    CuAssertStrEquals(tc, "filter_with_error failed with: 4"
                      " [file: noop, record: 1]",
                      crio_errmsg(stream));
    crio_stream_free(stream);
}

void Test_crio_copy_filename(CuTest *tc)
{
    char *fname = malloc(sizeof(char) * 9);
    strcpy(fname, "demo.txt");
    struct crio_stream *stream = crio_stream_make(demo_read, NULL,
                                                  fname, NULL);
    free(fname);
    fname = NULL;
    CuAssertStrEquals(tc, "demo.txt", stream->filename);
    crio_stream_free(stream);
}

void Test_crio_reset_file(CuTest *tc)
{
    struct crio_stream *stream = crio_stream_make(demo_read, NULL,
                                                  "demo.txt", NULL);
    stream->nread = 10;
    stream->nfiltered = 5;
    stream = crio_reset_file(stream, NULL, "second.txt");
    CuAssertStrEquals(tc, "second.txt", stream->filename);
    CuAssertIntEquals(tc, 0, stream->nread);
    CuAssertIntEquals(tc, 0, stream->nfiltered);
    crio_stream_free(stream);
}

void Test_no_filter_demo_crio(CuTest *tc)
{
    FILE *file = fopen("demo.txt", "r");
    char line[256];
    memset(line, 0, 256);
    struct crio_stream *stream = crio_stream_make(demo_read, file, "demo.txt", 
                                                  line);
    char *expect[] = {"abc", "def", "ab3fg", "123", "456", "4asdfds",
                      "ghi", "k4", "jkl"};
    int res, i = 0;
    while (CRIO_OK == (res = crio_next(stream))) {
        CuAssertTrue(tc, i < 9);
        CuAssertStrEquals(tc, expect[i], line);
        i++;
    }
    CuAssertIntEquals(tc, 9, i);
    CuAssertIntEquals(tc, CRIO_EOF, res);
    CuAssertIntEquals(tc, 9, stream->nread);
    CuAssertIntEquals(tc, 9, stream->nfiltered);
    fclose(file);
    crio_stream_free(stream);
}

void Test_one_filter_demo_crio(CuTest *tc)
{
    FILE *file = fopen("demo.txt", "r");
    char line[256];
    memset(line, 0, 256);
    struct crio_stream *stream = crio_stream_make(demo_read, file, "demo.txt", 
                                                  line);
    crio_add_filter(stream, "nodigits", alpha_filter, NULL);
    char *expect[] = {"abc", "def", "ghi", "jkl"};
    int res, i = 0;
    while (CRIO_OK == (res = crio_next(stream))) {
        CuAssertTrue(tc, i < 4);
        CuAssertStrEquals(tc, expect[i], line);
        i++;
    }
    CuAssertIntEquals(tc, 4, i);
    CuAssertIntEquals(tc, CRIO_EOF, res);
    CuAssertIntEquals(tc, 9, stream->nread);
    CuAssertIntEquals(tc, 4, stream->nfiltered);
    fclose(file);
    crio_stream_free(stream);
}


void Test_two_filters_demo(CuTest *tc)
{
    FILE *file = fopen("demo.txt", "r");
    char line[256];
    memset(line, 0, 256);
    struct crio_stream *stream = crio_stream_make(demo_read, file, "demo.txt", 
                                                  line);
    crio_add_filter(stream, "nodigits", alpha_filter, NULL);
    crio_add_filter(stream, "has k", has_k_filter, NULL);
    CuAssertIntEquals(tc, 2, stream->filter_count);
    char *expect[] = {"jkl"};
    int res, i = 0;
    while (CRIO_OK == (res = crio_next(stream))) {
        CuAssertTrue(tc, i < 1);
        CuAssertStrEquals(tc, expect[i], line);
        i++;
    }
    CuAssertIntEquals(tc, 1, i);
    CuAssertIntEquals(tc, CRIO_EOF, res);
    CuAssertIntEquals(tc, 9, stream->nread);
    CuAssertIntEquals(tc, 1, stream->nfiltered);
    fclose(file);
    crio_stream_free(stream);
}
