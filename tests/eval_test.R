library("crio")

strstr_filter <- crio:::.strstr_filter
eval_test <- crio:::.eval_test

e <- new.env(parent=emptyenv(), hash = TRUE)
e[["a"]] <- strstr_filter("a")
e[["b"]] <- strstr_filter("b")
e[["c"]] <- strstr_filter("c")

eval_test(quote(a), e, "foo")
eval_test(quote(a), e, "a foo")

eval_test(quote(a & b), e, "a")
eval_test(quote(a & b), e, "abc")
eval_test(quote(a & b | c), e, "cd")
eval_test(quote((a | c) & b), e, "c")
eval_test(quote((a | c) & (b | a)), e, "a")
