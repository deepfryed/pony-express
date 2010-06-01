#!/usr/bin/ruby

require 'mkmf'

$CFLAGS = "-DHAVE_INTTYPES_H"

Config::CONFIG['CC']  = 'g++'
Config::CONFIG['CPP'] = 'g++'

dir_config("mimetic", ["/usr/local", "/opt/local", "/usr"])

headers = [ 'stdio.h', 'mimetic/mimetic.h' ]
if have_library('mimetic',  nil, headers)
  create_makefile 'mimetic'
else
  puts <<-ERROR
    Cannot find mimetic headers or libraries.
    Try sudo apt-get install libmimetic-dev on debian flavors of linux.
  ERROR
  exit 1
end
