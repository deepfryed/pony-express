require 'net/smtp'
require 'shellwords'
require 'pony-express/mimetic'

module PonyExpress
  TRANSPORTS           = [ :smtp, :sendmail ]
  DEFAULT_SMTP_OPTIONS = { host: 'localhost', port: '25', domain: 'localhost.localdomain' }
  @@sendmail_binary    = '/usr/sbin/sendmail'

  Mimetic.load_mime_types File.join(File.dirname(__FILE__), '..', 'mime.types')

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
    via         = via.to_sym

    envelope_from = options[:from]

    if headers = options[:headers]
      if return_path = headers.find {|h| h[:name] == 'Return-Path'}
        envelope_from = return_path[:value]
      elsif sender = headers.find {|h| h[:name] == 'Sender'}
        envelope_from = sender[:value]
      end
    end

    if TRANSPORTS.include? via
      case via
        when :sendmail then transport_via_sendmail build(options), envelope_from, options[:to], via_options
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

  def transport_via_sendmail content, from, to, options = {}
    IO.popen([sendmail_binary, '-t', '-i', "-f #{Shellwords.escape(from)}"], 'w') {|io| io.write(content)}
  end

  def transport_via_smtp content, from, to, options = {}
    o = DEFAULT_SMTP_OPTIONS.merge(options)
    smtp = Net::SMTP.new(o[:host], o[:port])
    if o[:tls]
      unless smtp.respond_to?(:enable_starttls)
        begin
          require 'smtp_tls'
        rescue LoadError
          raise "You may need: gem install smtp_tls"
        end
      end
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

    def initialize options = {}
      @options = options
    end


    # Add an option.
    #
    # @example add.
    #  require "pony-express"
    #  mail = PonyExpress::Mail.new
    #  mail.add to: "burns@plant.local"
    def add options
      @options.merge! options
    end

    # Remove an option.
    #
    # @example remove.
    #  require "pony-express"
    #  mail = PonyExpress::Mail.new
    #  mail.add to: "burns@plant.local", cc: "smithers@plant.local"
    #  mail.remove :cc
    def remove keys
      keys = keys.map(&:to_sym)
      @options.reject! {|k, v| keys.include?(k) }
    end

    # Send the email via the selected transport.
    #
    def dispatch
      mail(@options)
    end
    def deliver; dispatch end

    # Return the encoded email content.
    #
    def content
      build(@options)
    end
  end

  extend self
end
