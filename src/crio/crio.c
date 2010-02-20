#include "crio.h"
#include <stdlib.h>

struct crio_stream *
crio_stream_make(int (*read)(void *fh, void *ctx),
                 int (*filter)(void *ctx),
                 void *fh)
{
    struct crio_stream *stream = malloc(sizeof(struct crio_stream));
    if (stream) {
        stream->read = read;
        stream->filter = filter;
        stream->file = fh;
    }
    return stream;
}

int crio_stream_next(struct crio_stream *stream, void *ctx)
{
    int res;
    /* TODO: should we add validity checking for stream fields? E.g.: */
    /* if (!stream || !stream->file || ... etc) return CRIO_ERR */
    while (CRIO_OK == (res = stream->read(stream->file, ctx))) {
        if (stream->filter(ctx)) break;
    }
    return res;
}
