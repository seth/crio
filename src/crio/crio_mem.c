#include "crio_mem.h"
#include <err.h>
#include <stdarg.h>

static void
crio_panic(const char *fmt, ...)
{
    va_list(ap);
    va_start(ap, fmt);
    verrx(1, fmt, ap);
    va_end(ap);
}

static void *
(*_allocator)(size_t s) = malloc;

static void
(*_releaser)(void *p) = free;

static void
(*_panic)(const char *, ...) = crio_panic;

void
crio_mpool_init(
    void * (*allocator)(size_t),
    void (*releaser)(void *),
    void (*panic)(const char *, ...))
{
    if (allocator) _allocator = allocator;
    if (releaser) _releaser = releaser;
    if (panic) _panic = panic;
}

struct _crio_mpool *
crio_mpool_make(size_t init_size)
{
    struct _crio_mpool *pool = _allocator(sizeof(struct _crio_mpool));
    if (!pool) _panic("crio_mpool_make: no memory");
    pool->size = init_size;
    pool->data = _allocator(init_size);
    if (!pool->data)
        _panic("crio_mpool_make: no memory for pool (%d)", init_size);
    pool->used = 0;
    pool->next = NULL;
    return pool;
}

void *
crio_mpool_alloc(struct _crio_mpool *pool, size_t s)
{
    int avail = (pool->size - pool->used);
    /* FIXME: we should allocate a new pool and link in */
    if (s > avail) _panic("crio_mpool_alloc: no space for %d", s);
    void * mem = pool->data + (pool->used);
    pool->used += s;
    return mem;
}

size_t
crio_mpool_mark(struct _crio_mpool *pool)
{
    return pool->used;
}

void
crio_mpool_drain_to_mark(struct _crio_mpool *pool,
                         size_t mark)
{
    pool->used = mark;
}

void
crio_mpool_drain(struct _crio_mpool *pool)
{
    pool->used = 0;
}

void
crio_mpool_free(struct _crio_mpool *pool)
{
    _releaser(pool->data);
    _releaser(pool);
    pool = NULL;
}
