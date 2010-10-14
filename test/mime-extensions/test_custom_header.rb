# encoding: utf-8
require_relative '../helper'

describe "Custom Header" do
  before do
    @mail = PonyExpress::Mail.new from: "user1@example.com", to: "user2@example.com", subject: "test"
    @mail.add text: "hello", html: "hello world", headers: [{name: "X-FooBar", value: "blarg"}]
  end

  it "should generate custom header" do
    assert_match %r{^X-FooBar: blarg\r$}m, @mail.content
  end

  it "should raise error if headers is not an array" do
    assert_raises(ArgumentError) do
      @mail.add headers: "foo"
      @mail.content
    end
  end

  it "should raise runtime if headers contains something thats not a hash" do
    assert_raises(RuntimeError) do
      @mail.add headers: ["foo"]
      @mail.content
    end
  end
end

