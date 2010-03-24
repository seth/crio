dummy_filter <- function(s, ctx)
{
    .Call(make_dummy_filter, s, ctx)
}

strstr_filter <- function(s)
{
    .Call(make_strstr_filter, s)
}

build_ast <- function(expr, rho)
{
    invisible(.Call(crio_build_ast, expr, rho))
}

eval_test <- function(expr, rho, ctx)
{
    .Call(crio_build_and_eval_ast, expr, rho, ctx)
}
