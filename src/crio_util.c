#include "crio_util.h"
#include <string.h>

/* Return 1 if the external pointer has a tag slot matching the
 * specified prefix, 0 otherwise.
 */
int crio_is_valid_xp(SEXP xp, const char *prefix)
{
    if (TYPEOF(xp) == EXTPTRSXP) {
        SEXP tag = R_ExternalPtrTag(xp);
        if (isString(tag) && LENGTH(tag) == 1) {
            const char *t = CHAR(STRING_ELT(tag, 0));
            if (strstr(t, prefix)) return 1; /* OK */
        }
    }
    return 0;
}

/* Check that the specified R external pointer has a tag slot matching
 * the specified prefix.  An error is thrown if not.
 *
 * This provides some protection against the wrong external pointer
 * being passed to crio routines.
 */
void crio_check_xp(SEXP xp, const char *prefix)
{
    if (!crio_is_valid_xp(xp, prefix))
        Rf_error("external pointer is not a valid %s", prefix);
}
