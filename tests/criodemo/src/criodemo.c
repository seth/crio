#include <R.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>

#include <crio_stubs.c>

void R_init_criodemo(DllInfo *info)
{
    crio_initialize_stubs();
}

