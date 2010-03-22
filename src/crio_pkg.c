#include "crio_pkg.h"

#define PKG "crio"

#define REG_FUNC(X) R_RegisterCCallable(PKG, (#X), (DL_FUNC)&(X))

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
    REG_FUNC(crio_set_filters);
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
    struct crio_stream *cs = (struct crio_stream *) R_ExternalPtrAddr(xp);
    crio_stream_free(cs);
    R_ClearExternalPtr(xp);
}

SEXP crio_stream_make_xp(int (*read)(struct crio_stream *stream),
                         void *fh,
                         char *filename,
                         void *ctx)
{
    SEXP xp;
    struct crio_stream *cs = crio_stream_make(read, fh, filename, ctx);
    PROTECT(xp = R_MakeExternalPtr(cs, mkString("crio_stream"),
                                   R_NilValue));
    R_RegisterCFinalizerEx(xp, crio_stream_xp_free, 0);
    UNPROTECT(1);
    return xp;
    
}

SEXP crio_reset_file_xp(SEXP xp, void *fh, char *filename)
{
    struct crio_stream *cs = (struct crio_stream *) R_ExternalPtrAddr(xp);
    R_SetExternalPtrAddr(xp, crio_reset_file(cs, fh, filename));
    return xp;
}

int crio_next_xp(SEXP xp)
{
    struct crio_stream *cs = (struct crio_stream *) R_ExternalPtrAddr(xp);
    return crio_next(cs);
}

void crio_set_errmsg_xp(SEXP xp, const char *fmt, ...)
{
    struct crio_stream *cs = (struct crio_stream *) R_ExternalPtrAddr(xp);
    va_list(ap);
    va_start(ap, fmt);
    crio_vset_errmsg(cs, fmt, ap);
    va_end(ap);
}

const char * crio_errmsg_xp(SEXP xp)
{
    struct crio_stream *cs = (struct crio_stream *) R_ExternalPtrAddr(xp);
    return crio_errmsg(cs);
}



