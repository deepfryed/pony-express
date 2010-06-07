require_relative '../helper'

describe "Basic Mail Header" do
  before do
    mail = PonyExpress::Mail.new from: "user1@example.com", to: "user2@example.com", subject: "test message"
    mail.add text: "hello", html: "hello world"
    @content = mail.content
  end

  it "should generate basic header information correctly" do
    assert_match %r{From: user1@example.com}, @content, "FROM address"
    assert_match %r{To: user2@example.com}, @content, "TO address"
    assert_match %r{Subject: test message}, @content, "subject"
  end
end
