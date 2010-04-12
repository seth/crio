tmp_lib <- tempfile()
dir.create(tmp_lib)
install.packages("criodemo", repos = NULL, lib = tmp_lib)
library("criodemo", lib.loc = tmp_lib)

example(read_demo)


filters <- list(
                a = substr_filter("a"),
                b = substr_filter("b"),
                c = substr_filter("c"),
                q = substr_filter("q"),
                u = substr_filter("u"))

f <- "/usr/share/dict/words"
if (file.exists(f)) {
    ab_words <- read_demo(f, "a | b", filters)
    stopifnot(all(grepl("a|b", ab_words)))
    q_words <- read_demo(f, "q", filters)
    stopifnot(all(grepl("q", q_words)))
    q_no_u_words <- read_demo(f, "q & !u", filters)
    stopifnot(all(grepl("q", q_no_u_words)))
    stopifnot(!any(grepl("u", q_no_u_words)))
}



## force gc to test finalizers
gc()

detach("package:criodemo", unload = TRUE, character.only = TRUE)
unlink(tmp_lib, recursive = TRUE)
