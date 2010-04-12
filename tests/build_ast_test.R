library("crio")

dummy_filter <- crio:::.dummy_filter
build_ast <- crio:::.build_ast

e <- new.env(parent=emptyenv(), hash = TRUE)
e[["a"]] <- dummy_filter("aa", 1)
e[["b"]] <- dummy_filter("bb", 2)
e[["c"]] <- dummy_filter("cc", 3)
e[["d"]] <- dummy_filter("dd", 4)

build_ast(quote(a), e)
build_ast(quote(a & b), e)
build_ast(quote(a & b | c), e)
build_ast(quote((a | c) & b), e)
build_ast(quote((a | c) & (b | a)), e)
build_ast(quote((a | (b & c)) & (d)), e)
