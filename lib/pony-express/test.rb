module PonyExpress
  module Test
    def mailbox
      PonyExpress::Test.mailbox
    end

    def self.mailbox
      @@mailbox ||= []
    end
  end

  def self.transport_via_sendmail content, *rest
    PonyExpress::Test.mailbox << content
  end

  def self.transport_via_smtp content, *rest
    PonyExpress::Test.mailbox << content
  end
end
