#include <Rinternals.h>
#include <R_ext/Rdynload.h>
#include "crio/crio.h"

#define PKG "crio"

#define REG_FUNC(X) R_RegisterCCallable(PKG, (#X), (DL_FUNC)&(X))

void R_init_crio(DllInfo *info)
{
    REG_FUNC(crio_stream_make);
    REG_FUNC(crio_stream_free);
    REG_FUNC(crio_reset_file);
    REG_FUNC(crio_add_filter);
    REG_FUNC(crio_next);
    REG_FUNC(crio_set_errmsg);
    REG_FUNC(crio_errmsg);
}
