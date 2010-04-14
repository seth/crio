#include <R.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>

#include <crio_xp.h>

/* Example filter function for use with crio
 *
 * All filter functions share the same signature.  The 'stream'
 * argument can be used to access the stream's context data via
 * stream->ctx.  Typically, this is how the filter function will
 * access the data for the current record.  In this package, the
 * context is simply a char * buffer.
 *
 * The 'fctx' argument provides access to filter-specific context.
 * When using the crio_xp.h API, fctx will always be a SEXP and can be
 * used to parameterize your filter.  Here, the filter context is used
 * to store what substring the filter should search for.
 *
 * Filter functions should return one of the following three integer
 * values: CRIO_FILT_PASS, CRIO_FILT_FAIL, or CRIO_ERR.
 * 
 */
static int strstr_filter(struct crio_stream *stream, void *fctx)
{
    char *buf = (char *)stream->ctx;
    const char *s = CHAR(STRING_ELT((SEXP)fctx, 0));
    char *found = strstr(buf, s);
    return found ? CRIO_FILT_PASS : CRIO_FILT_FAIL;
}

/* .Call entry point for criodemo to create a strstr filter
 * 
 * This function returns an external pointer wrapping a crio filter.
 * The filter will match records that contain the string specified in
 * the character vector passed in by the user in 'ctx'.
 */
SEXP _make_strstr_filter(SEXP ctx)
{
    const char fmt[] = "strstr_filter(\"%s\")";
    const char *s = CHAR(STRING_ELT(ctx, 0));
    int n = strlen(s) + strlen(fmt) + 1;
    char *buf = R_alloc(n, sizeof(char));
    snprintf(buf, n, fmt, s);
    return crio_filter_make_xp(buf,
                               strstr_filter,
                               ctx);
}

/* Another example of a crio filter function
 *
 * This one ignores the current record.
 *
 */
static int dummy_filter(struct crio_stream *stream, void *fctx)
{
    int pass = LOGICAL((SEXP)fctx)[0];
    return pass ? CRIO_FILT_PASS : CRIO_FILT_FAIL;
}

SEXP _make_dummy_filter(SEXP ctx)
{
    int v;
    if (!Rf_isLogical(ctx)) {
        Rf_error("dummy filter context must be a logical vector");
    }
    v = LOGICAL(ctx)[0];
    return crio_filter_make_xp(v ? "dummy_TRUE" : "dummy_FALSE",
                               dummy_filter, ctx);
}

/* Example crio read function 
 *
 * When you create a crio stream using crio_stream_make_xp, you need
 * to specify a function that will read/parse records from the
 * stream's file.
 *
 * The read function should read a record from stream->file and put
 * the result in stream->ctx.  The read function should return one of
 * the following three integer values: CRIO_OK, CRIO_EOF, CRIO_ERR.
 *
 * In this example, the stream's file object (stream->file) is a FILE
 * assumed to refer to a text file that can be read line at a time.
 * The stream's context is a char * buffer.
 *
*/
static int line_reader(struct crio_stream *stream)
{
    FILE *file = (FILE *)stream->file;
    char *buf = (char *)stream->ctx;
    int c = fscanf(file, "%s", buf);
    if (c == 1) return CRIO_OK;
    if (c == -1) return CRIO_EOF;
    return CRIO_ERR;
}

/* criodemo main .Call entry point
 * 
 * This function filters a specified text file according to a logical
 * combination of filter functions provided by the user.  It returns a
 * character vector of lines from the file that pass the filter.
 *
 * '_fname': a character vector specifying the filename to read from
 *
 * 'expr': An R expression describing the logical combination of
 * filters.  See read_demo.R for how a character vector at the R level can
 * be put in the right form.
 *
 * 'rho': An R environment mapping the symbols used in 'expr' to
 * external pointers representing crio filters as returned by
 * crio_filter_make_xp.
 *
 */
SEXP _demo_filter_file(SEXP _fname, SEXP expr, SEXP rho)
{
    int i = 0, ans_len = 256, res;
    PROTECT_INDEX pidx;
    SEXP ans, xp;
    const char *fname = CHAR(STRING_ELT(_fname, 0));
    FILE *fh = fopen(fname, "r");
    char *buf = R_alloc(256, sizeof(char));
    memset(buf, 0, 256);

    /* Here we create a new crio stream.  In this example, the stream
     * is created and destroyed (via gc) in a single .Call call.  You can also
     * separate the creation and use of a crio stream external pointer
     * into separate .Call functions.  This would make sense if you
     * want to process the returned data at the R level in chunks.
     */
    PROTECT(xp = crio_stream_make_xp(line_reader, (void *)fh, fname,
                                     (void *)buf,
                                     expr, rho));
    PROTECT_WITH_INDEX(ans = Rf_allocVector(STRSXP, ans_len), &pidx);

    /* This is the main processing loop.  After each call to
     * crio_next_xp, the context (here buf), will contain the next
     * record from the file that passes the filter.  A single call to
     * crio_next_xp will result in possibly many calls to your read
     * function until a record passing the filters (or EOF) is found.
     *
     * If crio_next_xp returns CRIO_ERR, you can obtain a (possibly)
     * useful error message by calling crio_errmsg_xp.  Your read and
     * filter functions can set a custom error message using
     * crio_set_errmsg (also crio_set_errmsg_xp).
     */
    while (CRIO_OK == (res = crio_next_xp(xp))) {
        if (i == ans_len) {
            /* grow the answer buffer */
            ans_len *= 2;
            REPROTECT(ans = lengthgets(ans, ans_len), pidx);
        }
        SET_STRING_ELT(ans, i, mkChar(buf));
        i++;
    }
    REPROTECT(ans = lengthgets(ans, i), pidx);
    UNPROTECT(2);
    fclose(fh);
    return ans;
}
