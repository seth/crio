#ifndef CRIO_H_
#define CRIO_H_

#define CRIO_OK   0
#define CRIO_EOF -1
#define CRIO_ERR -2

#define CRIO_FILT_FAIL  0
#define CRIO_FILT_PASS  1

struct crio_filter {
    char* name;
    int (*filter)(void *ctx, void *filter_ctx);
    void *filter_ctx;
    struct crio_filter *next;
};

struct crio_stream {
    void *file;
    int (*read)(void *file, void *ctx);
    struct crio_filter *filters;
    int filter_count;
    void *ctx;
};


/* crio_stream_make initializes a new stream.  You must free the
   returned crio_stream pointer when finished using crio_stream_free.
   Returns NULL if allocation fails.  */
/* TODO: should _make accept a pointers to malloc and free
 * functions? */
struct crio_stream *
crio_stream_make(int (*read)(void *fh, void *ctx),
                 void *fh,
                 void *ctx);

/* release memory for a crio_stream.  The stream is not usable
 * afterwards.
 */
void crio_stream_free(struct crio_stream *stream);


/* Add a filter to a stream.  Filters are composed with AND, executed
 * in a LIFO order; if the first filter executed returns 0, no other
 * filters will be run for that record.
 *
 * The contents of 'name' are copied to internal memory.  'filter' is
 * a function pointer taking global context and filter-specific
 * context.  'filter_ctx' is the filter specific context. Returns a
 * pointer to the stream or NULL if allocation fails. */
struct crio_stream *
crio_add_filter(struct crio_stream *stream,
                       const char *name,
                       int (*filter)(void *ctx, void *filter_ctx),
                       void *filter_ctx);

/* user code will call crio_next in a loop to process the
   stream */
int crio_next(struct crio_stream *);

#endif  /* CRIO_H_ */
