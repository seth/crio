                                README
                                ======

Date: 2010-04-12 14:34:32 PDT


Table of Contents
=================
1 crio package README 
    1.1 Why crio? 
    1.2 How does crio work? 
    1.3 Integrating crio into your package 


1 crio package README 
~~~~~~~~~~~~~~~~~~~~~~

1.1 Why crio? 
==============

The crio package helps R package developers implement stream-based
filtering on large un-indexed data files.  Without crio, a typical
workflow involves reading data into R, possibly in batches, and
performing filtering operations at the R level.  In some cases, the
overhead of reading the data into R can be substantial.  The crio
framework provides a means of executing filters written in C before
data is brought into R.

1.2 How does crio work? 
========================

To use the crio framework, a package creates a crio stream object
using crio's C API.  The stream encapsulates the file containing the
data, a function that knows how to read a record from the file, a
reference to a user supplied context object, and the desired filtering
operation.

Here's an example call:


  xp = crio_stream_make_xp(reader,
                           file,
                           fname,
                           context,
                           expr, rho);

where

- =reader= is a function that reads records from
  the file.

- =file= can be thought of as a file handle, but can be anything (a gz
  file handle, for example).

- =fname= is a label for =file=, used mostly for error reporting

- =context= is a user supplied context object that is threaded through
  all crio operations.  Generally, the =reader= function will store a
  record into a buffer in the context object and filter functions will
  look at the context to view the current record.

- =expr= is an R expression describing a logical combination of filter
  functions.  For example: =quote(a | (b & !c))=.

- =rho= is an R environment that maps the symbols used in =expr= to
  crio filter functions.

Once a crio stream is created, package code can call
=crio_stream_next_xp(stream)= to obtain the next record that passes
the filter.  What actually happens internally is that the reader function is
called until a record is found that passes the filter.  At this point,
the record passing filter is available in the =context= object for
further processing (conversion into R data structures, for example).

While the reader and filter functions must be written in C (and follow
the signature defined by the crio API), the framework allows for some
flexibility in filtering by allowing a user to specify a boolean
expression describing the final filter.  To make this concrete,
consider two filter functions =q= and =u=.  Filter =q= selects records
that contain the letter "q" and filter =u= selects records that
contain the letter "u".  Then words that contain the letter "q", but
no "u" could be obtain by specifying a filter expression of =q & !u=.

1.3 Integrating crio into your package 
=======================================

To integrate crio into your package, have a look at the criodemo
package provided in the tests subdirectory in a crio source package.
The criodemo package shows how to integrate crio to perform some
simple filtering operations on text files.
