#!/usr/bin/ruby

require 'mkmf'

$CFLAGS = "-DHAVE_INTTYPES_H"

RbConfig::CONFIG['CC']  = 'g++'
RbConfig::CONFIG['CPP'] = 'g++'

if !have_library('mimetic', 'memset')
  puts <<-ERROR

    Cannot find mimetic headers or libraries.
    Try sudo apt-get install libmimetic-dev on debian flavors of linux.

  ERROR
  exit 1
end

if !have_library('pcrecpp', 'memset')
  puts <<-ERROR

    Cannot find pcrecpp headers or libraries from pcre3.
    Try sudo apt-get install libpcre3-dev on debian flavors of linux.

  ERROR
  exit 1
end

create_makefile('mimetic')
