# encoding: utf-8
require 'helper'

describe "Quoted Printable" do
  before do
    @mail = PonyExpress::Mail.new from: "user1@example.com", to: "user2@example.com"
    @mail.add text: "hello", html: "hello world"
  end

  it "should generate quoted printable subject" do
    @mail.add subject: "hello© world?="
    assert_match 'Subject: =?UTF-8?Q?hello=C2=A9=20world=3F=3D?=', @mail.content
  end

  it "should only optional quote subject" do
    @mail.add subject: "hello world!"
    assert_match 'Subject: hello world!', @mail.content
  end
end

