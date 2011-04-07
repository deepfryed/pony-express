require_relative '../helper'

describe "Mail Parser" do
  before do
    @mail = File.read(File.dirname(__FILE__) + '/test.msg')
  end

  # TODO parser is WIP
  it "should parse content" do
    content = PonyExpress.parse(@mail)
    assert content
  end
end
