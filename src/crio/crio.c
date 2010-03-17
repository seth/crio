#include "crio.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

static struct crio_stream *
copy_stream_filename(struct crio_stream *stream,
                     const char *filename)
{
    if (stream) {
        char *fname = NULL;
        if (filename) {
            int len = strlen(filename) + 1;
            fname = malloc(sizeof(char) * len);
            if (!fname) {
                crio_stream_free(stream);
                return NULL;
            }
            memcpy(fname, filename, len);
        }
        stream->filename = fname;
    }
    return stream;
}

struct crio_stream *
crio_stream_make(int (*read)(struct crio_stream *stream),
                 void *fh,
                 char *filename,
                 void *ctx)
{
    struct crio_stream *stream = malloc(sizeof(struct crio_stream));
    char *fname = NULL;
    if (filename) {
        int len = strlen(filename) + 1;
        fname = malloc(sizeof(char) * len);
        memcpy(fname, filename, len);
    }
    if (stream) {
        stream->read = read;
        stream->file = fh;
        stream->ctx = ctx;
        stream->filters = NULL;
        stream->filter_count = 0;
        stream->nfiltered = 0;
        stream->nread = 0;
    }
    return copy_stream_filename(stream, filename);
}

static void crio_filter_free(struct crio_filter *cf)
{
    /* recursive free of chained filters */
    if (cf) {
        crio_filter_free(cf->next);
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
    crio_filter_free(stream->filters);
    if (stream->filename) free(stream->filename);
    free(stream);
    stream = NULL;
}


struct crio_stream *
crio_reset_file(struct crio_stream *stream,
                void *fh,
                char *filename)
{
    free(stream->filename);
    stream->filename = NULL;
    stream->file = fh;
    /* returns NULL if allocation fails */
    stream = copy_stream_filename(stream, filename);
    if (stream) {
        stream->nread = 0;
        stream->nfiltered = 0;
    }
    return stream;
}

struct crio_stream *
crio_add_filter(struct crio_stream *stream,
                const char *name,
                int (*filter)(struct crio_stream *stream, void *filter_ctx),
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


int crio_next(struct crio_stream *stream)
{
    int res, i, fres;
    struct crio_filter *cf;
    while (++(stream->nread) && CRIO_OK == (res = stream->read(stream))) {
        i = 0;
        for (cf = stream->filters; cf; cf = cf->next) {
            fres = cf->filter(stream, cf->filter_ctx);
            if (CRIO_FILT_FAIL == fres) break;
            else if (CRIO_FILT_PASS == fres) i++;
            else return fres; /* must be error */
        }
        if (i == stream->filter_count) {
            stream->nfiltered++;
            break;
        }
    }
    if (res == CRIO_EOF) stream->nread--;
    return res;
}

void crio_set_errmsg(struct crio_stream *stream, const char *fmt, ...)
{
    int len;
    va_list(ap);
    va_start(ap, fmt);
    vsnprintf(stream->error_message, CRIO_ERRBUF_SIZE, fmt, ap);
    va_end(ap);
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

const char * crio_errmsg(struct crio_stream *stream)
{
    return (const char *)stream->error_message;
}
