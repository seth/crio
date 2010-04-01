single_filter <- function(fname)
{
    system.time(z <- read_demo(fname, "d",
                               filters = list(d = dummy_filter(FALSE))))
}

multi_filter <- function(fname)
{
    filts <- list(
                  a = dummy_filter(TRUE),
                  b = dummy_filter(TRUE),
                  c = dummy_filter(FALSE))
    expr <- "(a & (b | c)) & ((a & c) | (b & c))"
    system.time(z <- read_demo(fname, expr, filters = filts))

}

make_file <- function(N=10000, W=100, seed = 0x8bba, f = NULL)
{
    set.seed(seed)
    data_lines <- make_data(N, W)
    tf <- if (is.null(f)) tempfile() else f
    writeLines(data_lines, con = tf)
    tf
}
