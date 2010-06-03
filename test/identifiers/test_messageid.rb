require_relative '../helper'

describe "Message-ID" do
  before do
    mail = PonyExpress::Mail.new from: "user1@example.com", to: "user2@example.com", subject: "test"
    mail.add text: "hello", html: "hello world"
    @content = mail.content
  end

  it "should generate Message-ID information correctly" do
    assert_match %r{Message-ID: <.*?>}, @content, "Message-ID"
  end
end

