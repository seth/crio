#include "crio.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "crio_eval.h"

static struct crio_stream *
copy_stream_filename(struct crio_stream *stream,
                     const char *filename)
{
    if (stream) {
        stream->filename = NULL;
        if (filename) {
            stream->filename = strdup(filename);
            if (!stream->filename) return NULL;
        }
    }
    return stream;
}

struct crio_stream *
crio_stream_make(int (*read)(struct crio_stream *stream),
                 void *file,
                 const char *filename,
                 void *ctx,
                 CrioNode filter)
{
    struct crio_stream *stream = malloc(sizeof(struct crio_stream));
    if (stream) {
        stream->read = read;
        stream->file = file;
        stream->ctx = ctx;
        stream->filter = filter;
        stream->nfiltered = 0;
        stream->nread = 0;
    }
    return copy_stream_filename(stream, filename);
}

void crio_filter_free(struct crio_filter *cf)
{
    if (cf) {
        if (cf->name) free(cf->name);
        if (cf->finalizer && cf->filter_ctx) {
            cf->finalizer(cf->filter_ctx);
            cf->filter_ctx = NULL;
            cf->finalizer = NULL;
        }
        free(cf);
        cf = NULL;
    }
}

void crio_stream_free(struct crio_stream *stream)
{
    if (!stream) return;
    if (stream->filename) free(stream->filename);
    free(stream);
    stream = NULL;
}

struct crio_stream *
crio_reset_file(struct crio_stream *stream,
                void *file,
                const char *filename)
{
    stream->file = file;
    if (stream->filename) free(stream->filename);
    /* returns NULL if allocation fails */
    stream = copy_stream_filename(stream, filename);
    if (stream) {
        stream->nread = 0;
        stream->nfiltered = 0;
    }
    return stream;
}

struct crio_filter *
crio_filter_make(const char *name,
                 int (*filter)(struct crio_stream *stream, void *filter_ctx),
                 void *filter_ctx,
                 void (*finalizer)(void *))
{
    struct crio_filter *cf;
    if (!(cf = malloc(sizeof(struct crio_filter)))) return NULL;
    if (!(cf->name = calloc(strlen(name) + 1, sizeof(char)))) return NULL;
    strcpy(cf->name, name);
    cf->filter = filter;
    cf->filter_ctx = filter_ctx;
    cf->finalizer = finalizer;
    return cf;
}

CrioNode crio_combine_filters(int n, ...)
{
    int i;
    struct crio_filter *cf;
    CrioList *list = NULL;

    va_list(ap);
    va_start(ap, n);
    for (i = 0; i < n; i++) {
        cf = va_arg(ap, struct crio_filter *);
        list = crio_cons(crio_mknode_filter(cf), list);
    }
    va_end(ap);
    return crio_mknode_list(crio_cons(crio_mknode_fun_and(), list));
}

int crio_list_length(CrioList *list);

int crio_next_with_pool(struct crio_stream *stream, struct _crio_mpool *pool)
{
    int res, fres = CRIO_FILT_PASS, new_pool = pool ? 0 : 1;
    struct _crio_mpool *prev_pool = NULL;
    size_t mark;
    CrioList *filter_ast = stream->filter ? CRIO_LIST(stream->filter) : NULL;
    if (filter_ast && new_pool) {
        int len = crio_list_length(filter_ast);
        size_t pool_size = sizeof(struct _crio_node) * len * 4;
        if (pool_size < 256) pool_size = 256;
        pool = crio_mpool_make0(pool_size);
        prev_pool = crio_get_global_mem_pool();
        crio_set_global_mem_pool(pool);
    }
    while (++(stream->nread) && CRIO_OK == (res = stream->read(stream))) {
        if (filter_ast) {
            mark = crio_mpool_mark(pool);
            fres = CRIO_VALUE(_crio_eval(filter_ast, stream));
            crio_mpool_drain_to_mark(pool, mark);
        }
        /* filter pass or error */
        if (CRIO_FILT_PASS == fres || CRIO_FILT_FAIL != fres) break;
    }
    res == CRIO_EOF ? stream->nread-- : stream->nfiltered++;
    if (filter_ast && new_pool) {
        crio_set_global_mem_pool(prev_pool);
        crio_mpool_free(pool);
    }
    return fres == CRIO_ERR ? fres : res;
}

int crio_next(struct crio_stream *stream)
{
    return crio_next_with_pool(stream, NULL);
}

void crio_vset_errmsg(struct crio_stream *stream, const char *fmt, va_list ap)
{
    int len;
    vsnprintf(stream->error_message, CRIO_ERRBUF_SIZE, fmt, ap);
    len = strlen(stream->error_message);
    char *fname = stream->filename ? stream->filename : "";
    /* max size of record count is ceil(log10(2^64)) => 20.  Then we
     * need 17 for the template */
    int add_len = strlen(fname) + 20 + 17;
    if (add_len < CRIO_ERRBUF_SIZE - len - 1) {
        char *s = stream->error_message + len;
        snprintf(s, CRIO_ERRBUF_SIZE - len, 
                 " [file: %s, record: %d]", fname, stream->nread);
    }
}

void crio_set_errmsg(struct crio_stream *stream, const char *fmt, ...)
{
    va_list(ap);
    va_start(ap, fmt);
    crio_vset_errmsg(stream, fmt, ap);
    va_end(ap);
}

const char * crio_errmsg(struct crio_stream *stream)
{
    return (const char *)stream->error_message;
}

void *
crio_current_file(struct crio_stream *stream)
{
    return stream->file;
}
