# -*- encoding: utf-8 -*-

Gem::Specification.new do |s|
  s.name = "pony-express"
  s.version = "0.9.2"

  s.required_rubygems_version = Gem::Requirement.new(">= 0") if s.respond_to? :required_rubygems_version=
  s.authors = ["Bharanee Rathna"]
  s.date = "2012-11-18"
  s.description = "A fast and lightweight mailer for ruby that uses libmimetic for generating mails"
  s.email = "deepfryed@gmail.com"
  s.extensions = ["ext/pony-express/extconf.rb"]
  s.files = ["mime.types", "lib/pony-express/test.rb", "lib/pony-express.rb", "ext/pony-express/mimetic.cxx", "ext/pony-express/extconf.rb", "README.md"]
  s.homepage = "http://github.com/deepfryed/pony-express"
  s.require_paths = ["lib", "ext"]
  s.rubygems_version = "1.8.24"
  s.summary = "A fast and lightweight mailer"

  if s.respond_to? :specification_version then
    s.specification_version = 3

    if Gem::Version.new(Gem::VERSION) >= Gem::Version.new('1.2.0') then
      s.add_development_dependency(%q<rake>, [">= 0"])
    else
      s.add_dependency(%q<rake>, [">= 0"])
    end
  else
    s.add_dependency(%q<rake>, [">= 0"])
  end
end
