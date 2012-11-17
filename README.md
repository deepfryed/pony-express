# Pony Express

A fast and lightweight ruby mailer based on http://github.com/benprew/pony. For a more
historical perspective read [Pony Express](http://en.wikipedia.org/wiki/Pony_Express).

## Installation

```
  sudo apt-get install libmimetic-dev
  sudo apt-get install libpcre3-dev
  sudo gem install pony-express
```

### Dependencies

* Ruby >= 1.9.1
* rubygems >= 1.3.5
* ruby development libraries             (debian: ruby1.9.1-dev)
* mimetic >= 0.9.6 development libraries (debian: libmimetic-dev)
* pcre3 development libraries            (debian: libpcre3-dev)

## Usage

```ruby
  require "pony-express"
  mail = PonyExpress::Mail.new to: "burns@plant.local", from: "homer@home.local", via: "sendmail"
  mail.add cc: "smithers@plant.local", bcc: "carl@plant.local", replyto: "homer+work@home.local"
  mail.add subject: "Hello Mr.Burns", text: "Can I have more donuts ?", html: "<strong>More Dooooonuuuuts!</strong>"
  mail.add attachments: [ "/home/homer/donuts.png" ], headers: [{name: "X-FooBar", value: "test"}]
  mail.dispatch
```

Don't want to create new mail objects ? Just pass in all options to PonyExpress.mail

```ruby
  PonyExpress.mail to: "burns@plant.local", from: "homer@home.local", ...
```

## Testing

```ruby
  require "pony-express"
  require "pony-express/test"
  mail = PonyExpress::Mail.new to: "burns@plant.local", from: "homer@home.local", via: "sendmail"
  mail.add subject: "Hello Mr.Burns", text: "Can I have more donuts ?"
  mail.dispatch
  assert_equal 1, PonyExpress::Test.mailbox.length
```

## License

GPLv3
