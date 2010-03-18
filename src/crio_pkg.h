#ifndef CRIO_PKG_H_
#define CRIO_PKG_H_

#include <Rinternals.h>
#include <R_ext/Rdynload.h>
#include "crio/crio.h"

/* Create a crio filter wrapped in an external pointer.
 *
 * filter_ctx provides a mechanism to parameterize the behavior of a
 * given filter function.  In this case, the filter-specific context
 * must be a SEXP and will be protected by the returned external pointer.
 *
 */
SEXP crio_make_filter_xp(const char *name,
                         int (*filter)(struct crio_stream *stream, void *filter_ctx),
                         SEXP filter_ctx);

SEXP crio_make_stream_xp(int (*read)(struct crio_stream *stream),
                         void *fh,
                         char *filename,
                         void *ctx);


#endif  /* CRIO_PKG_H_ */
