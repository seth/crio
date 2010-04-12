## This is the main function of criodemo
##
## Filter a text file according to a boolean expression of filter functions.
##
## filename: string giving the name of the file to filter
##
## filter.expr: a string expressing a logical combination of filters.
## You can use '(', ')', '&', '|', and '!' to express a filter.
##
## filters: a named list of filters.  The names should correspond to the
## symbols in 'filter.expr'.  The values should be external pointers
## wrapping crio filter objects.
##
read_demo <-
function(filename, filter.expr, filters)
{
    filter_env <- new.env(parent = emptyenv(), hash = TRUE)
    for (nm in names(filters)) filter_env[[nm]] <- filters[[nm]]

    .Call(`_demo_filter_file`, path.expand(filename),
          parse(text=filter.expr)[[1L]],
          filter_env)
}

