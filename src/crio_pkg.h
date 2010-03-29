#ifndef CRIO_PKG_H_
#define CRIO_PKG_H_

#include <Rinternals.h>
#include <R_ext/Rdynload.h>
#include "crio.h"

/* Create a crio filter wrapped in an external pointer.
 *
 * filter_ctx provides a mechanism to parameterize the behavior of a
 * given filter function.  In this case, the filter-specific context
 * must be a SEXP and will be protected by the returned external pointer.
 *
 * The lifecycle of the filter is controlled by the external pointer,
 * when the external pointer is garbage collected, the crio data
 * structures will be finalized.
 */
SEXP
crio_filter_make_xp(
    const char *name,
    int (*filter)(struct crio_stream *stream, void *filter_ctx),
    SEXP filter_ctx);

/* Create a crio stream wrapped in an external pointer.
 * 
 * The advantage of the crio_*_xp functions is that finalization of
 * the crio data structures is handled automatically.  This is
 * particularly advantageous if an error occurs in R API functions as
 * it allows memory allocated by crio to be freed.
 */
SEXP
crio_stream_make_xp(
    int (*read)(struct crio_stream *stream),
    void *file,
    const char *filename,
    void *ctx,
    SEXP expr,
    SEXP rho);

/* Reset the file handle and name for a "wrapped" crio stream  */
SEXP
crio_reset_file_xp(
    SEXP xp,
    void *file,
    const char *filename);

/* Read from stream until a record passes filters or EOF */
int
crio_next_xp(
    SEXP xp);

/* Set the error message for the crio stream */
void
crio_set_errmsg_xp(
    SEXP xp,
    const char *fmt,
    ...);

/* Get the error message last set on the crio stream */
const char *
crio_errmsg_xp(
    SEXP xp);

/* Returns a pointer to the void * representing the stream context
   contained in the specified crio stream object.  */
void *
crio_context_from_xp(
    SEXP xp);

#endif  /* CRIO_PKG_H_ */
