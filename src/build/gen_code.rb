#!/usr/bin/env ruby

require 'erb'

class CodeGen
  def initialize(api)
    @api = api
  end

  def write_header(template_file, dest_file)
    template = ERB.new(open(template_file).read)
    puts "writing: #{dest_file}"
    open(dest_file, "w") { |f| f.write(template.result(binding)) }
  end

  def write_stubs(template_file, dest_file)
    template = ERB.new(open(template_file).read, 0, "%<>")
    puts "writing: #{dest_file}"
    open(dest_file, "w") { |f| f.write(template.result(binding)) }
  end

  def api_for(name)
    @api.find { |a| a[:name] == name }
  end

  # private

  def make_header_decl(p)
    ans = []
    ans << p[:return]
    ans << p[:name] + "("
    spc = " " * 4
    ans << p[:args].map { |a| spc + a }.join(",\n")
    ans[-1] = ans[-1] + ");"
    ans.join("\n")
  end
  
  def make_extern_fp_decl(p)
    spc = " " * 4
    ans = ["extern", p[:return], "(*#{p[:name]})(",
           p[:args].map { |a| spc + a }.join(",\n")]
    ans[-1] = ans[-1] + ");"
    ans.join("\n")
  end
  
  def init_func_ptr(p)
    args = p[:args].join(", ")
    spc = "\n" + (" " * 5)
    "INFP(#{p[:return]},#{spc}#{p[:name]},#{spc}(#{args}));\n"
  end
  
  def set_func_ptr(p)
    args = p[:args].join(", ")
    spc = "\n" + (" " * 9)
    "MKFP(#{p[:return]},#{spc}#{p[:name]},#{spc}(#{args}));\n"
  end
  
  def declare(name)
    make_header_decl(api_for(name))
  end
  
end

if !File.exist?("DESCRIPTION")
  abort("Please run from package top-level (DESCRIPTION file not found)")
end

api_file = ARGV[0]

load(api_file)

gen = CodeGen.new(API)
FILES.each do |desc|
  case desc[:type]
  when "header"
    gen.write_header(desc[:src], desc[:dst])
  when "stubs"
    gen.write_stubs(desc[:src], desc[:dst])
  else
    raise "unknown type: #{gen_type}, expecting 'header' or 'stubs'"
  end
end
