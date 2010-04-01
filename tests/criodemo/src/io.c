#include <R.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>

#include <crio_xp.h>

static int strstr_filter(struct crio_stream *stream, void *fctx)
{
    char *buf = (char *)stream->ctx;
    const char *s = CHAR(STRING_ELT((SEXP)fctx, 0));
    char *found = strstr(buf, s);
    return found ? CRIO_FILT_PASS : CRIO_FILT_FAIL;
}

SEXP _make_strstr_filter(SEXP ctx)
{
    const char fmt[] = "strstr_filter(\"%s\")";
    const char *s = CHAR(STRING_ELT(ctx, 0));
    int n = strlen(s) + strlen(fmt) + 1;
    char *buf = R_alloc(n, sizeof(char));
    snprintf(buf, n, fmt, s);
    return crio_filter_make_xp(buf,
                               strstr_filter,
                               ctx);
}

static int dummy_filter(struct crio_stream *stream, void *fctx)
{
    int pass = LOGICAL((SEXP)fctx)[0];
    return pass ? CRIO_FILT_PASS : CRIO_FILT_FAIL;
}

SEXP _make_dummy_filter(SEXP ctx)
{
    int v;
    if (!Rf_isLogical(ctx)) {
        Rf_error("dummy filter context must be a logical vector");
    }
    v = LOGICAL(ctx)[0];
    return crio_filter_make_xp(v ? "dummy_TRUE" : "dummy_FALSE",
                               dummy_filter, ctx);
}

static int line_reader(struct crio_stream *stream)
{
    FILE *file = (FILE *)stream->file;
    char *buf = (char *)stream->ctx;
    int c = fscanf(file, "%s", buf);
    if (c == 1) return CRIO_OK;
    if (c == -1) return CRIO_EOF;
    return CRIO_ERR;
}

SEXP _demo_filter_file(SEXP _fname, SEXP expr, SEXP rho)
{
    int i = 0, ans_len = 256, res;
    PROTECT_INDEX pidx;
    SEXP ans, xp;
    const char *fname = CHAR(STRING_ELT(_fname, 0));
    FILE *fh = fopen(fname, "r");
    char *buf = R_alloc(256, sizeof(char));
    memset(buf, 0, 256);
    PROTECT(xp = crio_stream_make_xp(line_reader, (void *)fh, fname,
                                     (void *)buf,
                                     expr, rho));
    PROTECT_WITH_INDEX(ans = Rf_allocVector(STRSXP, ans_len), &pidx);
    while (CRIO_OK == (res = crio_next_xp(xp))) {
        if (i == ans_len) {
            ans_len *= 2;
            REPROTECT(ans = lengthgets(ans, ans_len), pidx);
        }
        SET_STRING_ELT(ans, i, mkChar(buf));
        i++;
    }
    REPROTECT(ans = lengthgets(ans, i), pidx);
    UNPROTECT(2);
    fclose(fh);
    return ans;
}

