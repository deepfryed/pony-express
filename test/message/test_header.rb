# encoding: utf-8
require 'helper'

describe "Basic Mail Header" do
  before do
    mail = PonyExpress::Mail.new(
      from: "user1@example.com",
      to: "user2@example.com",
      subject: "Macht mich glücklich, zu verkünden, dass Mr. Weiße den Editor befördert wurde."
    )

    mail.add text: "hello", html: "hello world"
    @content = mail.content
  end

  it "should generate basic header information correctly" do
    assert_match %r{From: user1@example.com}, @content, "FROM address"
    assert_match %r{To: user2@example.com}, @content, "TO address"

    subject  = 'Subject: =?UTF-8?Q?'
    subject += 'Macht_mich_gl=C3=BCcklich,=20zu=20verk=C3=BCnden,=20dass=20'
    subject += 'Mr.=20Wei=C3=9Fe=20den=20Editor=20bef=C3=B6rdert=20wurde.?='

    assert_match subject, @content, "Mail subject"
  end
end
