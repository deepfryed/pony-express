require 'net/smtp'
begin; require 'smtp_tls'; rescue LoadError; end
require_relative '../ext/mimetic'

module PonyExpress
  TRANSPORTS = [ :smtp, :sendmail ]
  DEFAULT_SMTP_OPTIONS = { :host => 'localhost', :port => '25', :domain => 'localhost.localdomain' }

  def self.build options
    # TODO validation.
    Mimetic.build(options)
  end

  def self.send options
    via = options.delete(:via) || 'sendmail'
    via_options = options.delete(:via_options) || {}

    via = options.delete(:via) || :smtp
    if TRANSPORTS.include? via.to_sym
      case via
      when :sendmail then transport_via_sendmail build(options), via_options
      when :smtp     then transport_via_smtp build(options), options[:from], options[:to], via_options
      end
    else
      raise ArgumentError, ":via can be one of #{TRANSPORTS}"
    end
  end

  def self.sendmail_binary
    sendmail = `which sendmail`.chomp
    sendmail.empty? ? '/usr/sbin/sendmail' : sendmail
  end

  def self.transport_via_sendmail content, options={}
    IO.popen('-', 'w+') do |pipe|
      if pipe
        pipe.write(content)
      else
        exec(sendmail_binary, "-t")
      end
    end
  end

  def self.transport_via_smtp content, from, to, options={}
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
end
