require 'date'
require 'pathname'
require 'rake'
require 'rake/clean'
require 'rake/testtask'

$rootdir = Pathname.new(__FILE__).dirname
$gemspec = Gem::Specification.new do |s|
  s.name          = 'pony-express'
  s.version       = '0.9.0'
  s.summary       = 'A fast and lightweight mailer'
  s.description   = 'A fast and lightweight mailer for ruby that uses libmimetic for generating mails'
  s.email         = 'deepfryed@gmail.com'
  s.homepage      = 'http://github.com/deepfryed/pony-express'
  s.authors       = ['Bharanee Rathna']
  s.require_paths = %w(lib ext)
  s.extensions    = FileList['ext/**/extconf.rb']
  s.test_files    = FileList['test/**/*_test.rb']
  s.files         = FileList[
    'mime.types',
    'lib/**/*.rb',
    'ext/*.{h,c,cxx}',
    'README.md',
  ]

  s.add_development_dependency('rake')
end

desc 'Generate gemspec'
task :gemspec do
  $gemspec.date = Date.today
  File.open('%s.gemspec' % $gemspec.name, 'w') {|fh| fh.write($gemspec.to_ruby)}
end

desc 'compile extension'
task :compile do
  Dir.chdir('ext') do
    system('ruby extconf.rb && make -j2') or raise 'unable to compile pony-express'
  end
end

Rake::TestTask.new(:test) do |test|
  test.libs   << 'ext' << 'lib' << 'test'
  test.pattern = 'test/**/test_*.rb'
  test.verbose = true
end

task default: :test
task :test => [:compile]

desc 'tag release and build gem'
task :release => [:test, :gemspec] do
  system("git tag -m 'version #{$gemspec.version}' v#{$gemspec.version}") or raise "failed to tag release"
  system("gem build #{$gemspec.name}.gemspec")
end
