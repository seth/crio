#include <Rinternals.h>
#include <R_ext/Rdynload.h>
#include "crio_types.h"

#define PKG "crio"
#define DECL_FP(ret, name, args) ret (*name)args
#define CAST_FP(ret, args) (ret (*)args)
#define GET_FP(name) R_GetCCallable(PKG, #name)
#define INFP(ret, name, args) DECL_FP(ret, name, args) = NULL
#define MKFP(ret, name, args) name = CAST_FP(ret, args) GET_FP(name)


/* Declare API function pointers */
INFP(SEXP,
     crio_filter_make_xp,
     (const char *name, int (*filter)(struct crio_stream *stream, void *filter_ctx), SEXP filter_ctx));

INFP(SEXP,
     crio_stream_make_xp,
     (int (*read)(struct crio_stream *stream), void *file, const char *filename, void *ctx, SEXP expr, SEXP rho));

INFP(SEXP,
     crio_reset_file_xp,
     (SEXP xp, void *file, const char *filename));

INFP(int,
     crio_next_xp,
     (SEXP xp));

INFP(void,
     crio_set_errmsg_xp,
     (SEXP xp, const char *fmt, ...));

INFP(const char *,
     crio_errmsg_xp,
     (SEXP xp));

INFP(void *,
     crio_context_from_xp,
     (SEXP xp));

INFP(struct crio_stream *,
     crio_stream_make,
     (int (*read)(struct crio_stream *stream), void *file, const char *filename, void *ctx, CrioNode filter));

INFP(void,
     crio_stream_free,
     (struct crio_stream *stream));

INFP(struct crio_stream *,
     crio_reset_file,
     (struct crio_stream *stream, void *file, const char *filename));

INFP(struct crio_filter *,
     crio_filter_make,
     (const char *name, int (*filter)(struct crio_stream *stream, void *filter_ctx), void *filter_ctx, void (*finalizer)(void *filter_ctx)));

INFP(void,
     crio_filter_free,
     (struct crio_filter *));

INFP(CrioNode,
     crio_combine_filters,
     (int n, ...));

INFP(int,
     crio_next,
     (struct crio_stream *));

INFP(void,
     crio_set_errmsg,
     (struct crio_stream *stream, const char *fmt, ...));

INFP(void,
     crio_vset_errmsg,
     (struct crio_stream *stream, const char *fmt, va_list ap));

INFP(const char *,
     crio_errmsg,
     (struct crio_stream *stream));

INFP(void *,
     crio_current_file,
     (struct crio_stream *stream));


/* call this function inside YourPackage_init */
void crio_initialize_stubs()
{
    MKFP(SEXP,
         crio_filter_make_xp,
         (const char *name, int (*filter)(struct crio_stream *stream, void *filter_ctx), SEXP filter_ctx));

    MKFP(SEXP,
         crio_stream_make_xp,
         (int (*read)(struct crio_stream *stream), void *file, const char *filename, void *ctx, SEXP expr, SEXP rho));

    MKFP(SEXP,
         crio_reset_file_xp,
         (SEXP xp, void *file, const char *filename));

    MKFP(int,
         crio_next_xp,
         (SEXP xp));

    MKFP(void,
         crio_set_errmsg_xp,
         (SEXP xp, const char *fmt, ...));

    MKFP(const char *,
         crio_errmsg_xp,
         (SEXP xp));

    MKFP(void *,
         crio_context_from_xp,
         (SEXP xp));

    MKFP(struct crio_stream *,
         crio_stream_make,
         (int (*read)(struct crio_stream *stream), void *file, const char *filename, void *ctx, CrioNode filter));

    MKFP(void,
         crio_stream_free,
         (struct crio_stream *stream));

    MKFP(struct crio_stream *,
         crio_reset_file,
         (struct crio_stream *stream, void *file, const char *filename));

    MKFP(struct crio_filter *,
         crio_filter_make,
         (const char *name, int (*filter)(struct crio_stream *stream, void *filter_ctx), void *filter_ctx, void (*finalizer)(void *filter_ctx)));

    MKFP(void,
         crio_filter_free,
         (struct crio_filter *));

    MKFP(CrioNode,
         crio_combine_filters,
         (int n, ...));

    MKFP(int,
         crio_next,
         (struct crio_stream *));

    MKFP(void,
         crio_set_errmsg,
         (struct crio_stream *stream, const char *fmt, ...));

    MKFP(void,
         crio_vset_errmsg,
         (struct crio_stream *stream, const char *fmt, va_list ap));

    MKFP(const char *,
         crio_errmsg,
         (struct crio_stream *stream));

    MKFP(void *,
         crio_current_file,
         (struct crio_stream *stream));

}

