#ifndef CRIO_H_
#define CRIO_H_

#define CRIO_OK   0
#define CRIO_EOF -1
#define CRIO_ERR -2

#define CRIO_FILT_FAIL  0
#define CRIO_FILT_PASS  1

#define CRIO_ERRBUF_SIZE 1024

struct crio_stream {
    void *file;
    char *filename;
    int (*read)(struct crio_stream *stream);
    int nread;
    int nfiltered;
    struct crio_filter *filters;
    int filter_count;
    void *ctx;
    char error_message[CRIO_ERRBUF_SIZE];
};

struct crio_filter {
    char* name;
    int (*filter)(struct crio_stream *stream, void *filter_ctx);
    void *filter_ctx;
    struct crio_filter *next;
};



/* crio_stream_make initializes a new stream.  You must free the
   returned crio_stream pointer when finished using crio_stream_free.
   Returns NULL if allocation fails.  */
struct crio_stream *
crio_stream_make(int (*read)(struct crio_stream *stream),
                 void *fh,
                 char *filename,
                 void *ctx);

/* release memory allocated using crio_stream_make.  The stream is not
 * usable afterwards.
 */
void crio_stream_free(struct crio_stream *stream);

/* Set a new file handle and name on an existing stream object.  This
 * resets the counters used to keep track of number of reads and
 * number of records passing the filters. */
struct crio_stream *
crio_reset_file(struct crio_stream *stream,
                void *fh,
                char *filename);

/* Add a filter to a stream.  Filters are composed with AND, executed
 * in a LIFO order; if the first filter executed returns 0, no other
 * filters will be run for that record.
 *
 * The contents of 'name' are copied to internal memory.  'filter' is
 * a function pointer taking global context and filter-specific
 * context. Returns a pointer to the stream or NULL if allocation
 * fails. */
struct crio_stream *
crio_add_filter(struct crio_stream *stream,
                       const char *name,
                       int (*filter)(struct crio_stream *stream, void *filter_ctx),
                       void *filter_ctx);

/* Iterates over the stream.  The user-supplied read function is
   called until no records are left or a record passing the
   user-supplied filters is found. */
int crio_next(struct crio_stream *);

/* Set the error message on the specified stream.  The filename and
 * current record count context will be appended to the error message.
 * Error messages including context must be less than 1023 characters.
 * TODO: enhance this with va_list so messages can do easy string
 * interpolation */
void crio_set_errmsg(struct crio_stream *stream, const char *msg);

/* Return the current error message for a stream. */
const char * crio_errmsg(struct crio_stream *stream);

#endif  /* CRIO_H_ */
