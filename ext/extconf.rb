#!/usr/bin/ruby

require 'mkmf'

$CFLAGS = "-DHAVE_INTTYPES_H"

Config::CONFIG['CC']  = 'g++'
Config::CONFIG['CPP'] = 'g++'

if !have_library('mimetic')
  puts <<-ERROR

    Cannot find mimetic headers or libraries.
    Try sudo apt-get install libmimetic-dev on debian flavors of linux.

  ERROR
  exit 1
end

if !have_library('pcrecpp')
  puts <<-ERROR

    Cannot find pcrecpp headers or libraries from pcre3.
    Try sudo apt-get install libpcre3-dev on debian flavors of linux.

  ERROR
  exit 1
end

create_makefile('mimetic')
