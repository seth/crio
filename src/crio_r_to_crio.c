#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "crio_r_to_crio.h"
#include "crio/crio_eval.h"
#include "crio_pkg.h"

#define SYMCHAR(X) (CHAR(PRINTNAME((X))))

static int alpha_filter(struct crio_stream *stream, void *fctx)
{
    char *buf = (char *)stream->ctx;
    for ( ; buf[0]; buf++) {
        if (isdigit(buf[0])) return CRIO_FILT_FAIL;
    }
    return CRIO_FILT_PASS;
}

static int strstr_filter(struct crio_stream *stream, void *fctx)
{
    char *buf = (char *)stream->ctx;
    const char *s = CHAR(STRING_ELT((SEXP)fctx, 0));
    DEBUGPRINT("strstr: %s\n", s);
    char *found = strstr(buf, s);
    return found ? CRIO_FILT_PASS : CRIO_FILT_FAIL;
}

SEXP make_strstr_filter(SEXP ctx)
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

SEXP make_dummy_filter(SEXP name, SEXP ctx)
{
    return crio_filter_make_xp(CHAR(STRING_ELT(name, 0)),
                               alpha_filter,
                               ctx);
}

static int name_match(SEXP v, const char *name)
{
    if (TYPEOF(v) != SYMSXP)
        error("invalid type for name_match: %s",
              type2char(TYPEOF(v)));
    return strcmp(SYMCHAR(v), name) == 0;
}

static CrioNode _sym2CrioNode(SEXP s, SEXP rho, char **errmsg)
{
    CrioNode node = NULL;

    if (TYPEOF(s) != SYMSXP)
        error("invalid type for _sym2CrioNode: %s", type2char(TYPEOF(s)));
    if (CAR(s) == R_NilValue) return NULL;
    if (name_match(s, "&")) {
        node = crio_mknode_fun_and();
    } else if (name_match(s, "|")) {
        node = crio_mknode_fun_or();
    } else if (name_match(s, "!")) {
        node = crio_mknode_fun_not();
    } else {
        SEXP xp = Rf_findVarInFrame(rho, s);
        if (TYPEOF(xp) != EXTPTRSXP) {
            *errmsg = (char *)R_alloc(sizeof(char), 256);
            snprintf(*errmsg, 256, "invalid or missing binding for '%s' "
                     "in specified environment", SYMCHAR(s));
            return NULL;
        }
        struct crio_filter *cf = (struct crio_filter *)R_ExternalPtrAddr(xp);
        node = crio_mknode_filter(cf);
    }
    return node;
}

/* remove this to turn debugging ON */
#if 0
  #define DEBUG_AST(x, y) DEBUGPRINT(x, y)
#else
  #define DEBUG_AST(x, y)
#endif

/* Build a crio AST from the R expression in e.  Symbols are resolved
 * in the environment rho and are expected to be bound to external
 * pointers representing crio filters.  The resulting AST is stored in
 * out as a CrioNode of list type.
 *
 * Returns 0 if successful.  A return value of 1 indicates an error
 * occured in building the AST.  In this case, an error message is
 * available in errmsg.
 *
 */
int
_crio_R_to_ast(SEXP e, SEXP rho, CrioNode *out, char **errmsg)
{
    CrioNode tn1, tn2, tmp;
    if (e == R_NilValue) {
        *out = NULL;
        return 0;
    }
    switch (TYPEOF(e)) {
    case LANGSXP:
    case LISTSXP:
        switch (TYPEOF(CAR(e))) {
        case SYMSXP:
            DEBUG_AST("SYMSXP: %s\n", SYMCHAR(CAR(e)));
            if (name_match(CAR(e), "(")) {
                return _crio_R_to_ast(CAR(CDR(e)), rho, out, errmsg);
            }
            if (_crio_R_to_ast(CDR(e), rho, &tn1, errmsg)) return 1;
            if (NULL == (tmp = _sym2CrioNode(CAR(e), rho, errmsg)))
                return 1;
            *out = crio_mknode_list(crio_cons(tmp,
                                              tn1 ? CRIO_LIST(tn1) : NULL));
            break;
        case LANGSXP:
        case LISTSXP:
            DEBUG_AST("LANGSXP/LISTSXP, double recurse %d\n", 1);
            if (_crio_R_to_ast(CAR(e), rho, &tn1, errmsg)) return 1;
            if (_crio_R_to_ast(CDR(e), rho, &tn2, errmsg)) return 1;
            *out = crio_mknode_list(crio_cons(tn1,
                                              tn2 ? CRIO_LIST(tn2) : NULL));
            break;
        }
        break;
    case SYMSXP:
        DEBUG_AST("bare SYMSXP: %s\n", SYMCHAR(e));
        if (NULL == (tmp = _sym2CrioNode(e, rho, errmsg)))
            return 1;
        *out = crio_mknode_list(crio_cons(tmp, NULL));
        break;
    default:
        error("unhandled SEXP type in _crio_R_to_ast: %s",
              Rf_type2char(TYPEOF(e)));
    }
    return 0;
}

int crio_list2str(CrioList *, char *buf, size_t n);

SEXP crio_build_ast(SEXP expr, SEXP rho)
{
    CrioNode n;
    _crio_R_to_ast(expr, rho, &n, NULL);
    char *buf = (char *)R_alloc(256, sizeof(char));
    crio_list2str(CRIO_LIST(n), buf, 256);
    return mkString(buf);
}

SEXP crio_build_and_eval_ast(SEXP expr, SEXP rho, SEXP _ctx)
{
    CrioNode n;
    _crio_R_to_ast(expr, rho, &n, NULL);
    CrioList *list = CRIO_LIST(n);
    void *ctx = (void *)CHAR(STRING_ELT(_ctx, 0));
    struct crio_stream *stream = crio_stream_make(NULL, NULL,
                                                  "test stream", ctx,
                                                  NULL);
    return ScalarInteger(CRIO_VALUE(_crio_eval(list, stream)));
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

SEXP crio_filter_file(SEXP _fname, SEXP expr, SEXP rho)
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
