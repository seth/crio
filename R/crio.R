.dummy_filter <- function(s, ctx)
{
    .Call(.make_dummy_filter, s, ctx)
}

.strstr_filter <- function(s)
{
    .Call(.make_strstr_filter, s)
}

.build_ast <- function(expr, rho)
{
    .Call(.crio_build_ast, expr, rho)
}

.eval_test <- function(expr, rho, ctx)
{
    .Call(.crio_build_and_eval_ast, expr, rho, ctx)
}

.filter_file <- function(fname, expr, env)
{
    .Call(.crio_filter_file, path.expand(fname), expr, env)
}

.filter_file.r <- function(fname, pat, batch = -1) {
    if (batch <= 0) {
        lines <- readLines(fname)
        grep(pat, lines, value = TRUE)
    }
    else {
        con <- file(fname, open = "r")
        line_list <- list()
        i <- 1L
        while (length(lines <- readLines(con, n = batch)) > 0) {
            line_list[[i]] <- grep(pat, lines, value = TRUE)
            i <- i + 1L
        }
        close(con)
        unlist(line_list)
    }
}

.make_data <- function(nlines, word_len)
{
    mkWord <- function(x)
    {
        paste(sample(c(letters, LETTERS, as.character(0:9)),
                     x, replace = TRUE), collapse = "")
    }
    sapply(seq_len(nlines), function(x) mkWord(word_len))
}
