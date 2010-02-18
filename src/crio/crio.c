#include "crio.h"
#include <stdlib.h>

struct crio_stream *
crio_stream_make(int (*read_record)(void *fh, void *ctx),
                 int (*filter_record)(void *ctx),
                 void *fh)
{
    struct crio_stream *stream = malloc(sizeof(struct crio_stream));
    stream->read = read_record;
    stream->filter = filter_record;
    stream->file = fh;
    return stream;
}

int crio_stream_next(struct crio_stream *stream, void *ctx)
{
    int res;
    while (CRIO_OK == (res = stream->read(stream->file, ctx))) {
        if (stream->filter(ctx)) {
            return res;
        }
    }
    return res;
}
