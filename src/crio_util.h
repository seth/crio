#ifndef CRIO_UTIL_H_
#define CRIO_UTIL_H_

#include <Rinternals.h>

/* These prefixes are added to the tag slot of external pointers
 * wrapping crio filter and stream objects.  External pointers without
 * the expected prefix will be regarded as invalid.
 */
#define CRIO_XP_FILTER_PREFIX "crio filter"
#define CRIO_XP_STREAM_PREFIX "crio stream"

int crio_is_valid_xp(SEXP xp, const char *prefix);
void crio_check_xp(SEXP xp, const char *prefix);


#endif  /* CRIO_UTIL_H_ */
