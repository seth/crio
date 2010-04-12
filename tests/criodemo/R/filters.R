## create a substring filter that selects records (here lines of text
## from a file) that contain the substring specified in 's'.
##
## The return value is an external pointer wrapping a crio filter.
substr_filter <-
function(s)
{
    .Call(`_make_strstr_filter`, as.character(s)[1])
}

dummy_filter <- function(L)
{
    .Call(`_make_dummy_filter`, as.logical(L[1]))
}
