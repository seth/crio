
FILES = [
         {
           :type => "header",
           :src => "src/crio/crio_h.template",
           :dst => "src/crio/crio.h"
         },

         {
           :type => "stubs",
           :src => "src/crio/crio_stubs.template",
           :dst => "inst/include/crio_stubs.c"
         },

         {
           :type => "header",
           :src => "src/crio/crio_h.template",
           :dst => "inst/include/crio.h"
         },

         {
           :type => "header",
           :src => "src/crio_pkg_h.template",
           :dst => "inst/include/crio_xp.h"
         },

         {
           :type => "header",
           :src => "src/crio_pkg_h.template",
           :dst => "src/crio_pkg.h"
         }

        ]

API = [
       {
         :name => "crio_filter_make_xp",
         :return => "SEXP",
         :args =>
         ["const char *name",
          "int (*filter)(struct crio_stream *stream, void *filter_ctx)",
          "SEXP filter_ctx"]
       },

       {
         :name => "crio_stream_make_xp",
         :return => "SEXP",
         :args => ["int (*read)(struct crio_stream *stream)",
                   "void *fh",
                   "const char *filename",
                   "void *ctx",
                   "SEXP expr",
                   "SEXP rho"]
       },

       {
         :name => "crio_reset_file_xp",
         :return => "SEXP",
         :args => ["SEXP xp",
                   "void *fh",
                   "char *filename"]
       },

       {
         :name => "crio_next_xp",
         :return => "int",
         :args => ["SEXP xp"]
       },

       {
         :name => "crio_set_errmsg_xp",
         :return => "void",
         :args => ["SEXP xp", "const char *fmt", "..."]
       },

       {
         :name => "crio_errmsg_xp",
         :return => "const char *",
         :args => ["SEXP xp"]
       },

       {
         :name => "crio_stream_make",
         :return => "struct crio_stream *",
         :args => ["int (*read)(struct crio_stream *stream)",
                   "void *fh",
                   "const char *filename",
                   "void *ctx",
                   "CrioNode filter"]
       },

       {
         :name => "crio_stream_free",
         :return => "void",
         :args => ["struct crio_stream *stream"]
       },

       {
         :name => "crio_reset_file",
         :return => "struct crio_stream *",
         :args => ["struct crio_stream *stream",
                   "void *fh",
                   "char *filename"]
       },

       {
         :name => "crio_filter_make",
         :return => "struct crio_filter *",
         :args => ["const char *name",
                   "int (*filter)(struct crio_stream *stream, void *filter_ctx)",
                   "void *filter_ctx",
                   "void (*finalizer)(void *filter_ctx)"]
       },

       {
         :name => "crio_filter_free",
         :return => "void",
         :args => ["struct crio_filter *"]
       },

       {
         :name => "crio_combine_filters",
         :return => "CrioNode",
         :args => ["int n", "..."]
       },

       {
         :name => "crio_next",
         :return => "int",
         :args => ["struct crio_stream *"]
       },

       {
         :name => "crio_set_errmsg",
         :return => "void",
         :args => ["struct crio_stream *stream", "const char *fmt", "..."]
       },

       {
         :name => "crio_vset_errmsg",
         :return => "void",
         :args => ["struct crio_stream *stream", "const char *fmt", "va_list ap"]
       },

       {
         :name => "crio_errmsg",
         :return => "const char *",
         :args => ["struct crio_stream *stream"]
       }
      ]
