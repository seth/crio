library("crio")

strstr_filter <- crio:::.strstr_filter
filter_file <- crio:::.filter_file
filter_file.r <- crio:::.filter_file.r
make_data <- crio:::.make_data

e <- new.env(parent=emptyenv(), hash = TRUE)
e[["a"]] <- strstr_filter("a")
e[["b"]] <- strstr_filter("b")
e[["c"]] <- strstr_filter("c")

set.seed(0x8bba)
data_lines <- make_data(10000, 100)
tf <- tempfile()
on.exit(file.remove(tf))
writeLines(data_lines, con = tf)

z <- filter_file(tf, quote(a | b), e)
z.r <- filter_file.r(tf, "(a|b)")
stopifnot(all.equal(z, z.r))
length(z)


z <- filter_file(tf, quote(a | c), e)
z.r <- filter_file.r(tf, "(a|c)")
stopifnot(all.equal(z, z.r))
length(z)

gc()



