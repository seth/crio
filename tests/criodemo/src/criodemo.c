#include <R.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>

/* To use the crio C API, you need to include this "header" file in
   the source file where you define the R_init_yourpackage package
   initialization function.

   The entry points in crio are made available via the
   R_RegisterCCallable/R_GetCCallable mechanism.  To make this more
   convenient, the crio_stubs.c code declares the needed function
   pointers and the crio_initialize_stubs function initializes them by
   making the appropriate calls to R_GetCCallable.
  */
#include <crio_stubs.c>

void R_init_criodemo(DllInfo *info)
{
    /* This function is defined in crio_stubs.c, see the above include
     * directive.  It uses teh R_GetCCallable mechanism to initialize
     * function pointers that you will be able to use in your code to
     * access the crio API.
     */
    crio_initialize_stubs();
}

