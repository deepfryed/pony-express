require 'rubygems'
require 'rake'
require 'rake/clean'
require 'rake/testtask'
require 'rake/rdoctask'
require 'rake/extensiontask'

CLEAN << FileList[ 'ext/Makefile', 'ext/mimetic.so' ]

begin
  require 'jeweler'
rescue LoadError
  puts "Jeweler (or a dependency) not available. Install it with: gem install jeweler"
end

Jeweler::Tasks.new do |gem|
  gem.name        = 'pony-express'
  gem.summary     = 'A fast and lightweight mailer'
  gem.description = 'A fast and lightweight mailer for ruby that uses libmimetic for generating mails'
  gem.email       = 'deepfryed@gmail.com'
  gem.homepage    = 'http://github.com/deepfryed/pony-express'
  gem.authors     = ['Bharanee Rathna']
 
  gem.files = FileList[
    'lib/**/*.rb',
    'ext/*.{h,c,cxx}',
    'VERSION',
    'README'
  ]
  gem.extensions  = FileList[ 'ext/**/extconf.rb' ]
  gem.test_files  = FileList[ 'test/**/*_test.rb' ]
end

Jeweler::GemcutterTasks.new

Rake::ExtensionTask.new do |ext|
  ext.name    = 'mimetic'
  ext.ext_dir = 'ext'
  ext.lib_dir = 'ext'
end

Rake::RDocTask.new do |rdoc|
  version = File.exist?('VERSION') ? File.read('VERSION') : ""
  rdoc.rdoc_dir = 'rdoc'
  rdoc.title = "pony-express #{version}"
  rdoc.rdoc_files.include('README*')
  rdoc.rdoc_files.include('lib/**/*.rb')
end

Rake::TestTask.new(:test) do |test|
  test.libs << 'lib' << 'test'
  test.pattern = 'test/**/*_test.rb'
  test.verbose = true
end

task :test    => [ :compile, :check_dependencies ]
task :default => :test
