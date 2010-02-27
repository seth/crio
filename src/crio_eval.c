#include <stdio.h>
#include <string.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>
#include <stdlib.h>
/* findVarInFrame */

#define SYMCHAR(X) (CHAR(PRINTNAME((X))))

struct _CrioList {
    int value;
    struct _CrioList *next;
};

typedef struct _CrioList CrioList;

CrioList * crio_list_cons(int v, CrioList *list)
{
    CrioList *head = list;
    if (!list) {
        head = malloc(sizeof(CrioList));
        head->next = NULL;
        head->value = -1;
    }
    CrioList *e = malloc(sizeof(CrioList));
    /* FIXME error handle */
    e->value = v;
    e->next = head;
    return e;
}

void crio_list_print(CrioList *list)
{
    CrioList *e = list;
    for ( ; ; ) {
        if (!e) break;
        Rprintf("%d", e->value);
        if (e->next) Rprintf(", ");
        e = e->next;
    }
    Rprintf("\n");
}

void crio_list_free(CrioList *list)
{
    CrioList *cur = list, *tmp;
    while (cur) {
        tmp = cur;
        free(cur);
        cur = tmp->next;
    }
}

static int crio_and(int a, int b)
{
    return a && b;
}

static int crio_or(int a, int b)
{
    return a || b;
}

int crio_eval0(SEXP expr);

SEXP crio_eval(SEXP expr)
{
    return ScalarLogical(crio_eval0(expr));
}

static int handle_value(SEXP e)
{
    switch (TYPEOF(e)) {
    case INTSXP:
        return INTEGER(e)[0];
    case LGLSXP:
        return LOGICAL(e)[0];
    default:
        error("handle_value unexpected type: %s", type2char(TYPEOF(e)));
    }
}

static int name_match(SEXP v, const char *name)
{
    if (TYPEOF(v) != SYMSXP)
        error("invalid type for name_match: %s",
              type2char(TYPEOF(v)));
    return strcmp(SYMCHAR(v), "(") == 0;
}

int crio_eval0(SEXP expr)
{
    SEXP e = expr, v;
    v = CAR(e);
    switch (TYPEOF(e)) {
    case LANGSXP:
        if (TYPEOF(v) != SYMSXP) {
            Rf_PrintValue(v);
            error("CAR of LANGSXP was %s", type2char(TYPEOF(v)));
        }
        if (name_match(v, "(")) {
            return crio_eval0(CDR(e));
        } else {
            /* get function */
            Rprintf("lookup: %s\n", SYMCHAR(v));

            /* eval args */
            SEXP a = CDR(e);
            CrioList *list = NULL;
            while (a != R_NilValue) {
                int arg = crio_eval0(a);
                list = crio_list_cons(arg, list);
                a = CDR(a);
            }
            crio_list_print(list);
             /* crio_call(SYMCHAR(v), list); */
            break;
        }
    case SYMSXP:
        Rprintf("CAR(e) SYM: %s\n", SYMCHAR(CAR(e)));
        break;
    case LISTSXP:
        return crio_eval0(CAR(e));
    default:
        return handle_value(e);
    }
    return 0;
}

