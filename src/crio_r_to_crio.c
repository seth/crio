#include <stdio.h>
#include <string.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>
#include <stdlib.h>
#include "crio_pkg.h"
#include "crio/crio.h"
#include "crio/crio_eval.h"

#include <ctype.h>

/* findVarInFrame */

#define SYMCHAR(X) (CHAR(PRINTNAME((X))))

static int alpha_filter(struct crio_stream *stream, void *fctx)
{
    char *buf = (char *)stream->ctx;
    for ( ; buf[0]; buf++) {
        if (isdigit(buf[0])) return CRIO_FILT_FAIL;
    }
    return CRIO_FILT_PASS;
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

static CrioNode * _sym2CrioNode(SEXP s, SEXP rho)
{
    CrioNode *node = NULL;

    if (TYPEOF(s) != SYMSXP)
        error("invalid type for _sym2CrioNode: %s", type2char(TYPEOF(s)));
    if (CAR(s) == R_NilValue) return NULL;
    if (name_match(s, "&")) {
        node = crio_mknode_fun_and();
    } else if (name_match(s, "|")) {
        node = crio_mknode_fun_or();
    /* } else if (name_match(s, "(")) { */
        
    } else {
        SEXP xp = Rf_findVarInFrame(rho, s);
        /* FIXME: error handling for v not found in rho */
        struct crio_filter *cf = (struct crio_filter *)R_ExternalPtrAddr(xp);
        node = crio_mknode_filter(cf);
    }
    return node;
}

/* Return a CrioNode of type CRIO_LIST_T */
CrioNode *
_crio_R_to_ast(SEXP e, SEXP rho)
{
    CrioNode *tn1, *tn2;
    if (e == R_NilValue) return NULL;
    switch (TYPEOF(e)) {
    case LANGSXP:
    case LISTSXP:
        switch (TYPEOF(CAR(e))) {
        case SYMSXP:
            DEBUGPRINT("SYMSXP: %s\n", SYMCHAR(CAR(e)));
            if (name_match(CAR(e), "(")) {
                return _crio_R_to_ast(CAR(CDR(e)), rho);
            }
            tn1 = _crio_R_to_ast(CDR(e), rho);
            return crio_mknode_list(
                crio_cons(_sym2CrioNode(CAR(e), rho),
                          tn1 ? CRIO_LIST(tn1) : NULL));
        case LANGSXP:
        case LISTSXP:
            DEBUGPRINT("LANGSXP/LISTSXP, double recurse %d\n", 1);
            tn1 = _crio_R_to_ast(CAR(e), rho);
            tn2 = _crio_R_to_ast(CDR(e), rho);
            return crio_mknode_list(crio_cons(tn1,
                                              tn2 ? CRIO_LIST(tn2) : NULL));
        }
    case SYMSXP:
        DEBUGPRINT("bare SYMSXP: %s\n", SYMCHAR(e));
        return crio_mknode_list(crio_cons(_sym2CrioNode(e, rho), NULL));
    default:
        error("unhandled SEXP type in _crio_R_to_ast: %s",
              Rf_type2char(TYPEOF(e)));
    }
    return NULL;                /* -Wall */
}

void crio_print_list(CrioList *);

SEXP crio_build_ast(SEXP expr, SEXP rho)
{
    CrioList *list = CRIO_LIST(_crio_R_to_ast(expr, rho));
    crio_print_list(list);
    return ScalarLogical(1);
}
