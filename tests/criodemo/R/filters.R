substr_filter <-
function(s)
{
    .Call(`_make_strstr_filter`, as.character(s)[1])
}

dummy_filter <- function(L)
{
    .Call(`_make_dummy_filter`, as.logical(L[1]))
}
