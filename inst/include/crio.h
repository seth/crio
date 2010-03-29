#ifndef CRIO_H_
#define CRIO_H_

#include "crio_types.h"
#include <stdarg.h>

/* crio_stream_make initializes a new stream.  You must free the
   returned crio_stream pointer when finished using crio_stream_free.
   Returns NULL if allocation fails.  */
struct crio_stream *
crio_stream_make(
    int (*read)(struct crio_stream *stream),
    void *file,
    const char *filename,
    void *ctx,
    CrioNode filter);


/* release memory allocated using crio_stream_make.  The stream is not
 * usable afterwards.
 */
void
crio_stream_free(
    struct crio_stream *stream);


/* Set a new file handle and name on an existing stream object.  This
 * resets the counters used to keep track of number of reads and
 * number of records passing the filters. */
struct crio_stream *
crio_reset_file(
    struct crio_stream *stream,
    void *file,
    const char *filename);

/* Create a new crio filter.
 *
 */
struct crio_filter *
crio_filter_make(
    const char *name,
    int (*filter)(struct crio_stream *stream, void *filter_ctx),
    void *filter_ctx,
    void (*finalizer)(void *filter_ctx));

/* Free a crio filter created using crio_filter_make
 */
void
crio_filter_free(
    struct crio_filter *);

/* Create a CrioNode that can be passed to crio_stream_make
 * representing the combination of the specified filters using AND.
 */
CrioNode
crio_combine_filters(
    int n,
    ...);

/* Iterates over the stream.  The user-supplied read function is
   called until no records are left or a record passing the
   user-supplied filters is found. */
int
crio_next(
    struct crio_stream *);


/* Set the error message on the specified stream.  The filename and
 * current record count context will be appended to the error message.
 * Error messages including context must be less than 1023 characters.
 * The fmt argument is treated as a format via sprintf along with any
 * additional arguments. */
void
crio_set_errmsg(
    struct crio_stream *stream,
    const char *fmt,
    ...);

/* Set error message from a function that recieves '...' args.
 *
 */
void
crio_vset_errmsg(
    struct crio_stream *stream,
    const char *fmt,
    va_list ap);

/* Return the current error message for a stream. */
const char *
crio_errmsg(
    struct crio_stream *stream);

#endif  /* CRIO_H_ */
