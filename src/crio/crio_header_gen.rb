#!/usr/bin/env ruby

require 'erb'

def make_header_decl(p)
  ans = []
  ans << p[:return]
  ans << p[:name] + "("
  spc = " " * 4
  ans << p[:args].map { |a| spc + a }.join(",\n")
  ans[-1] = ans[-1] + ");"
  ans.join("\n")
end

def api_for(api, name)
  api.find { |a| a[:name] == name }
end

def init_func_ptr(p)
  args = p[:args].join(", ")
  "INFP(#{p[:return]}, #{p[:name]}, (#{args}));"
end

def set_func_ptr(p)
  args = p[:args].join(", ")
  "MKFP(#{p[:return]}, #{p[:name]}, (#{args}));"
end

def declare(api, name)
  make_header_decl(api_for(api, name))
end

api = [
       {
         :return => "struct crio_stream *",
         :name => "crio_stream_make",
         :args => ["int (*read)(struct crio_stream *stream)",
                   "void *fh",
                   "char *filename",
                   "void *ctx"]
       },

       {
         :return => "void",
         :name => "crio_stream_free",
         :args => ["struct crio_stream *stream"]
       },

       {
         :return => "struct crio_stream *",
         :name => "crio_reset_file",
         :args => ["struct crio_stream *stream",
                   "void *fh",
                   "char *filename"]
       },
       
       {
         :return => "struct crio_stream *",
         :name => "crio_add_filter",
         :args => ["struct crio_stream *stream",
                   "const char *name",
                   "int (*filter)(struct crio_stream *stream, void *filter_ctx)",
                   "void *filter_ctx"]
       },

       {
         :return => "int",
         :name => "crio_next",
         :args => ["struct crio_stream *"]
       },
       {
         :return => "void",
         :name => "crio_set_errmsg",
         :args => ["struct crio_stream *stream", "const char *fmt", "..."]
       },

       {
         :return => "const char *",
         :name => "crio_errmsg",
         :args => ["struct crio_stream *stream"]
       }
      ]

crio_h_template = ERB.new(open("crio_h.template").read, 0, "%<>")
crio_stubs_template = ERB.new(open("crio_stubs.template").read, 0, "%<>")

fn = "crio.h"
puts "writing: #{fn}"
open(fn, "w") do |f|
  f.write(crio_h_template.result(binding))
end

fn = "../../inst/include/crio_stubs.c"
puts "writing: #{fn}"
open(fn, "w") do |f|
  f.write(crio_stubs_template.result(binding))
end

