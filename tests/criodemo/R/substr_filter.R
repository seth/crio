substr_filter <-
function(s)
{
    .Call(`_make_strstr_filter`, as.character(s)[1])
}

