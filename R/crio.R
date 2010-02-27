my_eval <- function(expr)
{
    .Call(crio_eval, expr)
}

t1 <- function()
{
    my_eval(quote(a))
}
