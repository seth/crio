#ifndef CRIO_R_TO_CRIO_H_
#define CRIO_R_TO_CRIO_H_

#include <Rinternals.h>
#include "crio/crio.h"


CrioNode _crio_R_to_ast(SEXP e, SEXP rho);

#endif  /* CRIO_R_TO_CRIO_H_ */
