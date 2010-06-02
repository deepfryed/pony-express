#!/usr/bin/ruby

require 'mkmf'

$CFLAGS = "-DHAVE_INTTYPES_H"

Config::CONFIG['CC']  = 'g++'
Config::CONFIG['CPP'] = 'g++'

headers_mimetic = [ 'stdio.h', 'mimetic/mimetic.h' ]
headers_pcrecpp = [ 'stdio.h', 'pcre++.h' ]

if !have_library('mimetic',  nil, headers_mimetic)
  puts <<-ERROR

    Cannot find mimetic headers or libraries.
    Try sudo apt-get install libmimetic-dev on debian flavors of linux.

  ERROR
  exit 1
end

if !have_library('pcre++',  nil, headers_pcrecpp)
  puts <<-ERROR

    Cannot find pcre++ headers or libraries.
    Try sudo apt-get install libpcre++-dev on debian flavors of linux.

  ERROR
  exit 1
end

create_makefile('mimetic')
