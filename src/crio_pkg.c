#include "crio_pkg.h"
#include "crio_r_to_crio.h"
#include <R_ext/RS.h>
#include "crio/crio_mem.h"
#include "crio/crio_eval.h"
#include "crio_util.h"

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
    REG_FUNC(crio_context_from_xp);

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
    REG_FUNC(crio_current_file);
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
    char *name_buf;
    char *fmt = CRIO_XP_FILTER_PREFIX ": %s";
    int len = strlen(name) + strlen(fmt) + 1;
    struct crio_filter *cf = crio_filter_make(name, filter, filter_ctx, NULL);
    name_buf = (char *)R_alloc(len, sizeof(char));
    snprintf(name_buf, len, fmt, name);
    PROTECT(xp = R_MakeExternalPtr(cf, mkString(name_buf), filter_ctx));
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
                         void *file,
                         const char *filename,
                         void *ctx,
                         SEXP expr,
                         SEXP rho)
{
    SEXP xp;
    struct _crio_mpool *pool;
    struct stream_pair *csxp = Calloc(1, struct stream_pair);
    size_t pool_size = length(expr) * 4;
    char *errmsg = NULL;
    CrioNode ast;
    if (pool_size < 256) pool_size = 256;
    crio_mpool_init(NULL, NULL, Rf_error);
    pool = crio_mpool_make(pool_size);
    csxp->pool = pool;
    crio_set_global_mem_pool(pool);
    if (_crio_R_to_ast(expr, rho, &ast, &errmsg)) {
        Free(csxp);
        crio_mpool_free(pool);
        if (errmsg)
            Rf_error("%s", errmsg);
        else
            Rf_error("unknown error in crio_stream_make_xp");
    }
    csxp->stream = crio_stream_make(read, file, filename, ctx, ast);
    PROTECT(xp = R_MakeExternalPtr(csxp, mkString(CRIO_XP_STREAM_PREFIX),
                                   R_NilValue));
    R_RegisterCFinalizerEx(xp, crio_stream_xp_free, 0);
    UNPROTECT(1);
    return xp;
    
}

SEXP crio_reset_file_xp(SEXP xp, void *file, const char *filename)
{
    struct stream_pair *csxp;
    crio_check_xp(xp, CRIO_XP_STREAM_PREFIX);
    csxp = (struct stream_pair *)R_ExternalPtrAddr(xp);
    csxp->stream = crio_reset_file(csxp->stream, file, filename);
    return xp;
}

/* FIXME: this belongs in a private header, would be nice to avoid
   exposing struct _crio_mpool to the public crio API.
 */
int crio_next_with_pool(struct crio_stream *stream, struct _crio_mpool *pool);

int crio_next_xp(SEXP xp)
{
    size_t mark;
    int ret;
    struct stream_pair *csxp;
    crio_check_xp(xp, CRIO_XP_STREAM_PREFIX);
    csxp = (struct stream_pair *)R_ExternalPtrAddr(xp);
    mark = crio_mpool_mark(csxp->pool);
    ret = crio_next_with_pool(csxp->stream, csxp->pool);
    crio_mpool_drain_to_mark(csxp->pool, mark);
    return ret;
}

void crio_set_errmsg_xp(SEXP xp, const char *fmt, ...)
{
    struct stream_pair *csxp;
    crio_check_xp(xp, CRIO_XP_STREAM_PREFIX);
    csxp = (struct stream_pair *)R_ExternalPtrAddr(xp);
    va_list(ap);
    va_start(ap, fmt);
    crio_vset_errmsg(csxp->stream, fmt, ap);
    va_end(ap);
}

const char * crio_errmsg_xp(SEXP xp)
{
    struct stream_pair *csxp;
    crio_check_xp(xp, CRIO_XP_STREAM_PREFIX);
    csxp = (struct stream_pair *)R_ExternalPtrAddr(xp);
    return crio_errmsg(csxp->stream);
}

void *
crio_context_from_xp(SEXP xp)
{
    struct stream_pair *csxp;
    crio_check_xp(xp, CRIO_XP_STREAM_PREFIX);
    csxp = (struct stream_pair *)R_ExternalPtrAddr(xp);
    return csxp->stream->ctx;
}
