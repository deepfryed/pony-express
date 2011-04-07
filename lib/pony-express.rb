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

  def parse content
    Mimetic.parse(content)
  end

  def mail options
    via         = options.delete(:via)         || :smtp
    via_options = options.delete(:via_options) || {}

    if TRANSPORTS.include? via
      case via
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

    def add opt
      @options.merge! opt
    end

    def remove opt
      keys = opt.keys
      @options.reject! {|k, v| keys.include?(k) }
    end

    def dispatch
      mail(@options)
    end

    def content
      build(@options)
    end
  end

  extend self
end
