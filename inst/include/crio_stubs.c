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
INFP(struct crio_stream *,
     crio_stream_make,
     (int (*read)(struct crio_stream *stream), void *fh, char *filename, void *ctx));

INFP(void,
     crio_stream_free,
     (struct crio_stream *stream));

INFP(struct crio_stream *,
     crio_reset_file,
     (struct crio_stream *stream, void *fh, char *filename));

INFP(struct crio_filter *,
     crio_filter_make,
     (const char *name, int (*filter)(struct crio_stream *stream, void *filter_ctx), void *filter_ctx, void (*finalizer)(void *filter_ctx)));

INFP(void,
     crio_filter_free,
     (struct crio_filter *));

INFP(struct crio_stream *,
     crio_add_filter,
     (struct crio_stream *stream, const char *name, int (*filter)(struct crio_stream *stream, void *filter_ctx), void *filter_ctx));

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


/* call this function inside YourPackage_init */
void crio_initialize_stubs()
{
    MKFP(struct crio_stream *,
         crio_stream_make,
         (int (*read)(struct crio_stream *stream), void *fh, char *filename, void *ctx));

    MKFP(void,
         crio_stream_free,
         (struct crio_stream *stream));

    MKFP(struct crio_stream *,
         crio_reset_file,
         (struct crio_stream *stream, void *fh, char *filename));

    MKFP(struct crio_filter *,
         crio_filter_make,
         (const char *name, int (*filter)(struct crio_stream *stream, void *filter_ctx), void *filter_ctx, void (*finalizer)(void *filter_ctx)));

    MKFP(void,
         crio_filter_free,
         (struct crio_filter *));

    MKFP(struct crio_stream *,
         crio_add_filter,
         (struct crio_stream *stream, const char *name, int (*filter)(struct crio_stream *stream, void *filter_ctx), void *filter_ctx));

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

}

