#ifndef CRIO_MEM_H_
#define CRIO_MEM_H_

#include <stdlib.h>

struct _crio_mpool {
    size_t size;
    size_t used;
    void *data;
    struct _crio_mpool *next;
};
typedef struct _crio_mpool MemPool;

void
crio_mpool_init(
    void * (*allocator)(size_t),
    void (*releaser)(void *),
    void (*panic)(const char *, ...));

struct _crio_mpool *
crio_mpool_make(size_t init_size);

size_t
crio_mpool_mark(struct _crio_mpool *pool);

void
crio_mpool_drain_to_mark(struct _crio_mpool *pool,
                         size_t mark);

void
crio_mpool_drain(struct _crio_mpool *pool);

void
crio_mpool_free(struct _crio_mpool *pool);

void *
crio_mpool_alloc(struct _crio_mpool *pool, size_t s);



#endif  /* CRIO_MEM_H_ */
