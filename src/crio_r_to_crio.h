#ifndef CRIO_R_TO_CRIO_H_
#define CRIO_R_TO_CRIO_H_

#include <Rinternals.h>
#include "crio/crio.h"


int _crio_R_to_ast(SEXP e, SEXP rho, CrioNode *out_node, char **errmsg);

#endif  /* CRIO_R_TO_CRIO_H_ */
