dummy_filter <- function(s, ctx)
{
    .Call(make_dummy_filter, s, ctx)
}

build_ast <- function(expr, rho)
{
    invisible(.Call(crio_build_ast, expr, rho))
}
