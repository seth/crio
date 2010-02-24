#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "CuTest.h"
#include "crio.h"

static int demo_read(void *fh, void *ctx)
{
    FILE *file = (FILE *)fh;
    char *buf = (char *)ctx;
    int c = fscanf(file, "%s", buf);
    if (c == 1) return CRIO_OK;
    if (c == -1) return CRIO_EOF;
    return CRIO_ERR;
}

static int demo_read_int(void *fh, void *ctx)
{
    FILE *file = (FILE *)fh;
    int *buf = (int *)ctx;
    int c = fscanf(file, "%d", buf);
    if (c == 1) return CRIO_OK;
    if (c == -1) return CRIO_EOF;
    return CRIO_ERR;
}

static int demo_filter(void *ctx, void *fctx)
{
    char *buf = (char *)ctx;
    for ( ; buf[0]; buf++) {
        if (isdigit(buf[0])) return CRIO_FILT_FAIL;
    }
    return CRIO_FILT_PASS;
}

void Test_demo_crio_err(CuTest *tc)
{
    /* trying to parse int from this file should fail */
    FILE *file = fopen("demoTest.c", "r");
    int score = 0;
    struct crio_stream *stream = crio_stream_make(demo_read_int, file, &score);
    int res = crio_stream_next(stream);
    CuAssertIntEquals(tc, CRIO_ERR, res);
    fclose(file);
    free(stream);
}

void Test_demo_crio(CuTest *tc)
{
    FILE *file = fopen("demo.txt", "r");
    char line[256];
    memset(line, 0, 256);
    struct crio_stream *stream = crio_stream_make(demo_read, file, line);
    crio_stream_add_filter(stream, "nodigits", demo_filter, NULL);
    char *expect[] = {"abc", "def", "ghi", "jkl"};
    int res, i = 0;
    while (CRIO_OK == (res = crio_stream_next(stream))) {
        CuAssertTrue(tc, i < 4);
        CuAssertStrEquals(tc, expect[i], line);
        i++;
    }
    CuAssertIntEquals(tc, 4, i);
    CuAssertIntEquals(tc, CRIO_EOF, res);
    fclose(file);
    free(stream);
}
