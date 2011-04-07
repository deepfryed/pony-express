require 'net/smtp'
begin; require 'smtp_tls'; rescue LoadError; end
require_relative '../ext/mimetic'

module PonyExpress
  TRANSPORTS           = [ :smtp, :sendmail ]
  DEFAULT_SMTP_OPTIONS = { host: 'localhost', port: '25', domain: 'localhost.localdomain' }
  @@sendmail_binary    = '/usr/sbin/sendmail'

  Mimetic.load_mime_types File.dirname(__FILE__) + "/../mime.types"

  def build options
    # TODO validation.
    Mimetic.build(options)
  end

  # NOTE: parse is very much WIP, YMMV
  def parse content
    Mimetic.parse(content)
  end

  # Build and dispatch email in one go.
  #
  # @example mail.
  #  require "pony-express"
  #  PonyExpress.mail to: "burns@plant.local", from: "homer@home.local", via: "sendmail",
  #                   subject: "Hello Mr.Burns", text: "Can I have more donuts ?",
  #                   html: "<strong>More Dooooonuuuuts!</strong>",
  #                   attachments: [ "/home/homer/donuts.png" ],
  #                   headers: [{name: "X-FooBar", value: "test"}]
  #
  # @param  [String]  to            Email address.
  # @param  [String]  from          Email address.
  # @param  [String]  subject       Subject (will be converted to quoted printable if needed).
  # @param  [String]  text          Plain text content.
  # @param  [String]  cc            Email address (optional).
  # @param  [String]  bcc           Email address (optional).
  # @param  [String]  replyto       Email address (optional).
  # @param  [String]  html          HTML content (optional).
  # @param  [Array]   attachments   List of email attachments (optional).
  # @param  [Array]   headers       List of email headers (optional).
  # @return [TrueClass or FalseClass]
  def mail options
    via         = options.delete(:via)         || :smtp
    via_options = options.delete(:via_options) || {}

    if TRANSPORTS.include? via
      case via.to_sym
        when :sendmail then transport_via_sendmail build(options), via_options
        when :smtp     then transport_via_smtp build(options), options[:from], options[:to], via_options
      end
    else
      raise ArgumentError, ":via can be one of #{TRANSPORTS}"
    end
  end

  def sendmail_binary= binary
    @@sendmail_binary = binary
  end

  def sendmail_binary
    @@sendmail_binary
  end

  def transport_via_sendmail content, options = {}
    IO.popen([sendmail_binary, '-t'], 'w') {|io| io.write(content)}
  end

  def transport_via_smtp content, from, to, options = {}
    o = DEFAULT_SMTP_OPTIONS.merge(options)
    smtp = Net::SMTP.new(o[:host], o[:port])
    if o[:tls]
      raise "You may need: gem install smtp_tls" unless smtp.respond_to?(:enable_starttls)
      smtp.enable_starttls
    end
    if o.include?(:auth)
      smtp.start(o[:domain], o[:user], o[:password], o[:auth])
    else
      smtp.start(o[:domain])
    end
    smtp.send_message content, from, to
    smtp.finish
  end

  class Mail
    include PonyExpress
    attr_accessor :options

    def initialize opt={}
      @options = opt
    end


    # Add an option.
    #
    # @example add.
    #  require "pony-express"
    #  mail = PonyExpress::Mail.new
    #  mail.add to: "burns@plant.local"
    def add opt
      @options.merge! opt
    end

    # Remove an option.
    #
    # @example remove.
    #  require "pony-express"
    #  mail = PonyExpress::Mail.new
    #  mail.add to: "burns@plant.local", cc: "smithers@plant.local"
    #  mail.remove :cc
    def remove opt
      keys = opt.keys
      @options.reject! {|k, v| keys.include?(k) }
    end

    # Send the email via the selected transport.
    #
    def dispatch
      mail(@options)
    end

    # Return the encoded email content.
    #
    def content
      build(@options)
    end
  end

  extend self
end
