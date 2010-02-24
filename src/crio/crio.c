#include "crio.h"
#include <stdlib.h>
#include <string.h>


struct crio_stream *
crio_stream_make(int (*read)(void *fh, void *ctx),
                 void *fh,
                 void *ctx)
{
    struct crio_stream *stream = malloc(sizeof(struct crio_stream));
    if (stream) {
        stream->read = read;
        stream->file = fh;
        stream->ctx = ctx;
        stream->filters = NULL;
    }
    return stream;
}

static void crio_filter_free(struct crio_filter *cf)
{
    /* recursive free of chained filters */
    if (cf) {
        crio_filter_free(cf->next);
        if (cf->name) free(cf->name);
        free(cf);
        cf = NULL;
    }
}

void crio_stream_free(struct crio_stream *stream)
{
    crio_filter_free(stream->filters);
    free(stream);
    stream = NULL;
}

struct crio_stream *
crio_stream_add_filter(struct crio_stream *stream,
                       const char *name,
                       int (*filter)(void *ctx, void *filter_ctx),
                       void *filter_ctx)
{
    struct crio_filter *cf;
    if (!(cf = malloc(sizeof(struct crio_filter)))) return NULL;
    if (!(cf->name = calloc(strlen(name) + 1, sizeof(char)))) return NULL;
    strcpy(cf->name, name);
    cf->filter = filter;
    cf->filter_ctx = filter_ctx;
    cf->next = stream->filters;
    stream->filters = cf;
    stream->filter_count += 1;
    return stream;
}


int crio_stream_next(struct crio_stream *stream)
{
    int res, i;
    struct crio_filter *cf;
    while (CRIO_OK == (res = stream->read(stream->file, stream->ctx))) {
        for (i = 0, cf = stream->filters; cf; cf = cf->next, i++) {
            if (!cf->filter(stream->ctx, cf->filter_ctx)) break;
        }
        if (i == stream->filter_count) break;
    }
    return res;
}
