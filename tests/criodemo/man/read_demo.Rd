\name{read_demo}
\alias{read_demo}

\title{Read and filter a file and demo use of crio
}
\description{
More soon.
}
\usage{
read_demo(filename, filter.expr, filters)
}

\arguments{
  \item{filename}{File path}
  \item{filter.expr}{A string. See example}
  \item{filters}{An environment}
}
\details{
More soon.
}
\value{
  Stuff
}
\author{
S. Falcon
}

\examples{
filters <- list(
                a = substr_filter("a"),
                b = substr_filter("b"),
                c = substr_filter("c"))

set.seed(0x8bba)
data_lines <- make_data(10000, 100)
tf <- tempfile()
writeLines(data_lines, con = tf)

stuff <- read_demo(tf, "a | b", filters)
file.remove(tf)
}
