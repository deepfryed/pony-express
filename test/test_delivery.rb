require 'etc'
require 'digest/md5'
require 'helper'

# TODO make these tests platform independent
# TODO add test for sendmail from field, including weird chars to test shell escaping
describe "Delivery" do
  before do
    user   = '%s@localhost' % Etc.getlogin
    @spool = "/var/spool/mail/#{Etc.getlogin}"
    @sign  = Digest::MD5.hexdigest("#{Time.now.to_f} #{rand}")
    @mail  = PonyExpress::Mail.new from: user, to: user, subject: "test"
    @mail.add text: @sign, html: "hello world"
  end

  after do
    File.open(@spool, 'w') {|io| io.write('')}
  end

  it 'raise errors on unknown delivery mechanism' do
    @mail.add via: 'snail mail'
    assert_raises(ArgumentError) { @mail.dispatch }
  end

  it 'delivers mail via smtp' do
    @mail.add via: 'smtp'
    assert @mail.dispatch
    sleep 0.5
    assert_match %r{#{@sign}}, File.read(@spool)
  end

  it 'delivers mail via sendmail' do
    @mail.add via: 'sendmail'
    assert @mail.dispatch
    sleep 0.5
    assert_match %r{#{@sign}}, File.read(@spool)
  end
end
