#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "CuTest.h"
#include "crio_mem.h"


void Test_mem_pool_basics(CuTest *tc)
{
    int i;
    crio_mpool_init(malloc, free, NULL);
    struct _crio_mpool *pool = crio_mpool_make(sizeof(double) * 50);
    CuAssertIntEquals(tc, 0, crio_mpool_mark(pool));

    double *dd1 = crio_mpool_alloc(pool, sizeof(double) * 25);
    double *dd2 = crio_mpool_alloc(pool, sizeof(double) * 25);

    /* verify we got different addresses */
    double *dd1p = dd1, *dd2p = dd2;
    for (i = 0; i < 25; i++) {
        CuAssertTrue(tc, dd1p != dd2p);
        dd1p++; dd2p++;
    }

    /* drain, then ask for same, addresses should match */
    crio_mpool_drain(pool);
    dd1p = crio_mpool_alloc(pool, sizeof(double) * 25);
    dd2p = dd1;
    for (i = 0; i < 25; i++) {
        CuAssertTrue(tc, dd1p == dd2p);
        dd1p++; dd2p++;
    }
    crio_mpool_free(pool);
}

struct my_test_struct {
    void *data;
    struct my_test_struct *next;
    int count;
    char *msg;
    long id;
};

void Test_mem_pool_different_sizes(CuTest *tc)
{
    int i;
    crio_mpool_init(malloc, free, NULL);
    struct _crio_mpool *pool = crio_mpool_make(sizeof(unsigned char) * 1024);

    struct my_test_struct *mts =
        crio_mpool_alloc(pool,
                         sizeof(struct my_test_struct) * 5);
    char *buf = crio_mpool_alloc(pool, sizeof(char) * 10);
    int *p = crio_mpool_alloc(pool, sizeof(int) * 4);
    double *d = crio_mpool_alloc(pool, sizeof(double) * 3);
    
    for (i = 0; i < 5; i++) {
        char *msg = crio_mpool_alloc(pool, sizeof(char) * 3);
        sprintf(msg, "m%d", i);
        mts[i].data = crio_mpool_alloc(pool, sizeof(int) * 2);
        memset(mts[i].data, 0, sizeof(int) * 2);
        mts[i].count = 2;
        mts[i].msg = msg;
        mts[i].id = 123L;
    }
    sprintf(buf, "%s", "123456789");
    for (i = 0; i < 4; i++) p[i] = 9;
    for (i = 0; i < 3; i++) d[i] = 321L;

    for (i = 0; i < 5; i++) {
        size_t mark = crio_mpool_mark(pool);
        char *msg = crio_mpool_alloc(pool, sizeof(char) * 3);
        sprintf(msg, "m%d", i);
        CuAssertStrEquals(tc, msg, mts[i].msg);
        CuAssertIntEquals(tc, 2, mts[i].count);
        CuAssertTrue(tc, mts[i].id == 123L);
        crio_mpool_drain_to_mark(pool, mark);
    }

    for (i = 0; i < 4; i++) CuAssertIntEquals(tc, 9, p[i]);
    for (i = 0; i < 3; i++) CuAssertTrue(tc, d[i] == 321L);
    CuAssertStrEquals(tc, "123456789", buf);
    crio_mpool_free(pool);
}
    
