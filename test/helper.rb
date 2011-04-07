$:.unshift File.dirname(__FILE__) + '/../lib'
require 'minitest/unit'
require 'minitest/spec'
require 'pony-express'

MiniTest::Unit.autorun
