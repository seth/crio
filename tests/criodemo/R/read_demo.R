read_demo <-
function(filename, filter.expr, filters)
{
    filter_env <- new.env(parent = emptyenv(), hash = TRUE)
    for (nm in names(filters)) filter_env[[nm]] <- filters[[nm]]

    .Call(`_demo_filter_file`, path.expand(filename),
          parse(text=filter.expr)[[1L]],
          filter_env)
}

