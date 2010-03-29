#include "crio_pkg.h"
#include "crio_r_to_crio.h"
#include <R_ext/RS.h>
#include "crio/crio_mem.h"
#include "crio/crio_eval.h"

#define PKG "crio"

#define REG_FUNC(X) R_RegisterCCallable(PKG, (#X), (DL_FUNC)&(X))

struct stream_pair {
    struct _crio_mpool *pool;
    struct crio_stream *stream;
};

void R_init_crio(DllInfo *info)
{
    REG_FUNC(crio_stream_make_xp);
    REG_FUNC(crio_reset_file_xp);
    REG_FUNC(crio_next_xp);
    REG_FUNC(crio_set_errmsg_xp);
    REG_FUNC(crio_errmsg_xp);
    REG_FUNC(crio_filter_make_xp);

    REG_FUNC(crio_stream_make);
    REG_FUNC(crio_stream_free);
    REG_FUNC(crio_reset_file);
    REG_FUNC(crio_next);
    REG_FUNC(crio_set_errmsg);
    REG_FUNC(crio_vset_errmsg);
    REG_FUNC(crio_errmsg);

    REG_FUNC(crio_filter_make);
    REG_FUNC(crio_filter_free);
    REG_FUNC(crio_combine_filters);
}

static void crio_filter_xp_free(SEXP xp)
{
    struct crio_filter *cf = (struct crio_filter *) R_ExternalPtrAddr(xp);
    crio_filter_free(cf);
    R_ClearExternalPtr(xp);
}

SEXP crio_filter_make_xp(const char *name,
                         int (*filter)(struct crio_stream *, void *),
                         SEXP filter_ctx)
{
    SEXP xp;
    struct crio_filter *cf = crio_filter_make(name, filter, filter_ctx, NULL);
    PROTECT(xp = R_MakeExternalPtr(cf, mkString(name), filter_ctx));
    R_RegisterCFinalizerEx(xp, crio_filter_xp_free, 0);
    UNPROTECT(1);
    return xp;
}

static void crio_stream_xp_free(SEXP xp)
{
    struct stream_pair *csxp = (struct stream_pair *)R_ExternalPtrAddr(xp);
    crio_stream_free(csxp->stream);
    crio_mpool_free(csxp->pool);
    Free(csxp);
    csxp = NULL;
    R_ClearExternalPtr(xp);
}

SEXP crio_stream_make_xp(int (*read)(struct crio_stream *stream),
                         void *fh,
                         const char *filename,
                         void *ctx,
                         SEXP expr,
                         SEXP rho)
{
    SEXP xp;
    struct _crio_mpool *pool;
    struct stream_pair *csxp = Calloc(1, struct stream_pair);
    size_t pool_size = sizeof(struct _crio_node) * length(expr) * 10;
    if (pool_size < 256) pool_size = 256;
    crio_mpool_init(NULL, NULL, Rf_error);
    pool = crio_mpool_make(pool_size);
    csxp->pool = pool;
    crio_set_global_mem_pool(pool);
    csxp->stream = crio_stream_make(read, fh, filename, ctx,
                                    _crio_R_to_ast(expr, rho));
    PROTECT(xp = R_MakeExternalPtr(csxp, mkString("crio_stream"),
                                   R_NilValue));
    R_RegisterCFinalizerEx(xp, crio_stream_xp_free, 0);
    UNPROTECT(1);
    return xp;
    
}

SEXP crio_reset_file_xp(SEXP xp, void *fh, char *filename)
{
    struct stream_pair *csxp = (struct stream_pair *)R_ExternalPtrAddr(xp);
    csxp->stream = crio_reset_file(csxp->stream, fh, filename);
    return xp;
}

int crio_next_xp(SEXP xp)
{
    struct stream_pair *csxp = (struct stream_pair *)R_ExternalPtrAddr(xp);
    size_t mark = crio_mpool_mark(csxp->pool);
    int ret = crio_next(csxp->stream);
    crio_mpool_drain_to_mark(csxp->pool, mark);
    return ret;
}

void crio_set_errmsg_xp(SEXP xp, const char *fmt, ...)
{
    struct stream_pair *csxp = (struct stream_pair *)R_ExternalPtrAddr(xp);
    va_list(ap);
    va_start(ap, fmt);
    crio_vset_errmsg(csxp->stream, fmt, ap);
    va_end(ap);
}

const char * crio_errmsg_xp(SEXP xp)
{
    struct stream_pair *csxp = (struct stream_pair *)R_ExternalPtrAddr(xp);
    return crio_errmsg(csxp->stream);
}



