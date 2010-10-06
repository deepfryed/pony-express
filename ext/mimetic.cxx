#include <ruby.h>
extern "C" {
    #include <ruby/encoding.h>
}
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <exception>
#include <algorithm>
#include <pcrecpp.h>
#include <mimetic/mimetic.h>
#include <map>

#define ID_CONST_GET rb_intern("const_get")
#define CONST_GET(scope, constant) (rb_funcall(scope, ID_CONST_GET, 1, rb_str_new2(constant)))
#define RUBY_ENCODING(str) string(rb_enc_get(str)->name)
#define VALUEFUNC(f) ((VALUE (*)(ANYARGS)) f)
#define FORCE_ENCODING(str,enc) rb_enc_associate(str, rb_to_encoding(enc)); ENC_CODERANGE_CLEAR(str);
#define TO_S(v)                    rb_funcall(v, rb_intern("to_s"), 0)
#define CSTRING(v)                 RSTRING_PTR(TYPE(v) != T_STRING ? TO_S(v) : v)

static VALUE rb_mMimetic;
static VALUE eRuntimeError;
static VALUE eArgumentError;
static VALUE rb_UTF8, rb_ASCII;

using namespace std;
using namespace mimetic;
using namespace pcrecpp;

map<string, string> MimeTypes;

/* RFC References

http://www.ietf.org/rfc/rfc2822.txt  Internet Message Format.
http://www.ietf.org/rfc/rfc2045.txt  MIME Extensions - Multipart related (Part One).
http://www.ietf.org/rfc/rfc2046.txt  MIME Extensions - Multipart related (Part Deux).
http://www.ietf.org/rfc/rfc2392.txt  Content-ID and Message-ID.

*/

// TODO
// angle brackets enclosing content and message ids should be part of libmimetic.
string content_id() {
    string cid = ContentId().str();
    return cid[0] == '<' ? cid : "<" + cid + ">";
}

string message_id(int n) {
    string mid = MessageId(n).str();
    return mid[0] == '<' ? mid : "<" + mid + ">";
}

string file_mime_type(string file) {
    string extn;
    RE("(?:.*)\\.([^\\.]+)$").FullMatch(file, &extn);
    transform(extn.begin(), extn.end(), extn.begin(), ::tolower);
    string mime = MimeTypes[extn];
    return mime.length() > 0 ? mime : "application/octet-stream";
}

bool mimetic_attach_file(MimeEntity *m, char* filename) {
    filebuf ifile;
    ostringstream encoded;
    ifile.open(filename, ios::in);
    if (ifile.is_open()) {
        istream is(&ifile);
        Attachment *at = new Attachment(filename, ContentType(file_mime_type(filename)));
        Base64::Encoder b64;
        ostreambuf_iterator<char> oi(encoded);
        istreambuf_iterator<char> ibegin(is), iend;
        encode(ibegin, iend, b64, oi);
        at->body().assign(encoded.str());
        m->body().parts().push_back(at);
        ifile.close();
        return true;
    }
    else {
        rb_raise(eRuntimeError, "Mimetic: Unable to read attachment file %s", filename);
    }
    return false;
}

string safe_rfc2047(string v) {
    RE("\\?").GlobalReplace("=3F", &v);
    RE("_").GlobalReplace("=5F", &v);
    RE(" ").GlobalReplace("_", &v);
    return v;
}

string quoted_printable(VALUE str) {
    VALUE ascii = rb_str_new2(CSTRING(str));
    FORCE_ENCODING(ascii, rb_ASCII);
    if (rb_str_cmp(ascii, str) != 0) {
        string raw = CSTRING(str);
        QP::Encoder qp;
        istringstream is(raw);
        ostringstream encoded;
        istreambuf_iterator<char> ibeg(is), iend;
        ostreambuf_iterator<char> oi(encoded);
        encode(ibeg, iend, qp, oi);
        return "=?" + RUBY_ENCODING(str) + "?Q?" + safe_rfc2047(encoded.str()) + "?=";
    }
    else {
        return string(CSTRING(str));
    }
}

// Exposed API

void rb_load_mime_types(VALUE self, VALUE filename) {
    string piece;
    char buffer[4096];
    vector<string> data;
    RE regex("(.+?)(?:[\\r\\n\\t]+|$)");
    ifstream file(CSTRING(filename), ios::in);
    if (file.is_open()) {
        while (!file.eof()) {
            file.getline(buffer, 4096);
            StringPiece input(buffer);
            while (regex.Consume(&input, &piece)) data.push_back(piece);
            for (int i = 1; i < data.size(); i++)
                MimeTypes[data[i]] = data[0];
            data.clear();
        }
        file.close();
    }
    else {
        rb_raise(eRuntimeError, "Mimetic: Unable to load mime.types");
    }
}

VALUE rb_mimetic_build(VALUE self, VALUE options) {
    ostringstream output;
    VALUE attachment;
    VALUE text    = rb_hash_aref(options, ID2SYM(rb_intern("text")));
    VALUE to      = rb_hash_aref(options, ID2SYM(rb_intern("to")));
    VALUE from    = rb_hash_aref(options, ID2SYM(rb_intern("from")));
    VALUE subject = rb_hash_aref(options, ID2SYM(rb_intern("subject")));

    if (text    == Qnil) rb_raise(eArgumentError, "Mimetic.build called without :text");
    if (from    == Qnil) rb_raise(eArgumentError, "Mimetic.build called without :from");
    if (to      == Qnil) rb_raise(eArgumentError, "Mimetic.build called without :to");
    if (subject == Qnil) rb_raise(eArgumentError, "Mimetic.build called without :subject");

    // optional fields
    VALUE mid     = rb_hash_aref(options, ID2SYM(rb_intern("message_id")));
    VALUE html    = rb_hash_aref(options, ID2SYM(rb_intern("html")));
    VALUE tcid    = rb_hash_aref(options, ID2SYM(rb_intern("text_content_id")));
    VALUE hcid    = rb_hash_aref(options, ID2SYM(rb_intern("html_content_id")));
    VALUE replyto = rb_hash_aref(options, ID2SYM(rb_intern("replyto")));
    VALUE cc      = rb_hash_aref(options, ID2SYM(rb_intern("cc")));
    VALUE bcc     = rb_hash_aref(options, ID2SYM(rb_intern("bcc")));
    VALUE files   = rb_hash_aref(options, ID2SYM(rb_intern("attachments")));

    if (mid != Qnil && TYPE(mid) != T_STRING)
        rb_raise(eArgumentError, "Mimetic.build expects :message_id to be a string");

    if (tcid != Qnil && TYPE(tcid) != T_STRING)
        rb_raise(eArgumentError, "Mimetic.build expects :text_content_id to be a string");

    if (hcid != Qnil && TYPE(hcid) != T_STRING)
        rb_raise(eArgumentError, "Mimetic.build expects :html_content_id to be a string");

    if (files != Qnil && TYPE(files) != T_ARRAY)
        rb_raise(eArgumentError, "Mimetic.build expects :attachments to be an array");

    VALUE errors  = Qnil;
    MimeEntity *message   = NULL;
    MimeEntity *html_part = NULL;
    MimeEntity *text_part = NULL;
    MimeVersion v1("1.0");

    try {
        message = new MimeEntity;
        if (html != Qnil && text != Qnil) {
            delete message;
            message = new MultipartAlternative;
            html_part = new MimeEntity;
            text_part = new MimeEntity;
            message->body().parts().push_back(text_part);
            message->body().parts().push_back(html_part);
            text_part->header().contentType("text/plain; charset=" + RUBY_ENCODING(text));
            text_part->header().contentTransferEncoding("8bit");
            text_part->header().contentId(tcid == Qnil ? content_id() + ">" : ContentId(CSTRING(tcid)));
            text_part->header().mimeVersion(v1);
            text_part->body().assign(CSTRING(text));
            html_part->header().contentType("text/html; charset=" + RUBY_ENCODING(html));
            html_part->header().contentTransferEncoding("7bit");
            html_part->header().contentId(hcid == Qnil ? content_id() + ">" : ContentId(CSTRING(hcid)));
            html_part->header().mimeVersion(v1);
            html_part->body().assign(CSTRING(html));
        }
        else {
            message->body().assign(CSTRING(text));
            message->header().contentType("text/plain; charset=" + RUBY_ENCODING(text));
            message->header().contentTransferEncoding("8bit");
            message->header().mimeVersion(v1);
        }

        if (files != Qnil && RARRAY_LEN(files) > 0) {
            MimeEntity *m = message;
            message = new MultipartMixed();
            message->header().mimeVersion(v1);
            message->body().parts().push_back(m);
            for (long i = 0; i < RARRAY_LEN(files); i++) {
                attachment = rb_ary_entry(files, i);
                if (attachment != Qnil && TYPE(attachment) == T_STRING)
                    mimetic_attach_file(message, CSTRING(attachment));
            }
        }

        message->header().from(CSTRING(from));
        message->header().to(CSTRING(to));
        message->header().subject(quoted_printable(subject));
        message->header().messageid(mid != Qnil ? CSTRING(mid) : message_id(1));

        if (replyto != Qnil) message->header().replyto(CSTRING(replyto));
        if (cc      != Qnil) message->header().cc(CSTRING(cc));
        if (bcc     != Qnil) message->header().bcc(CSTRING(bcc));

        output << *message << endl;
        delete message;
        return rb_str_new2(output.str().c_str());
    }
    catch(exception &e) {
        if (message   != NULL) delete message;
        errors = rb_str_new2(e.what());
    }
    catch (...) {
        if (message   != NULL) delete message;
        errors = rb_str_new2("Unknown Error");
    }

    rb_raise(eRuntimeError, "Mimetic boo boo : %s\n", CSTRING(errors));
}

extern "C"  {
    void Init_mimetic(void) {
        eRuntimeError  = CONST_GET(rb_mKernel, "RuntimeError");
        eArgumentError = CONST_GET(rb_mKernel, "ArgumentError");
        rb_mMimetic = rb_define_module("Mimetic");
        rb_define_module_function(rb_mMimetic, "build", VALUEFUNC(rb_mimetic_build), 1);
        rb_define_module_function(rb_mMimetic, "load_mime_types", VALUEFUNC(rb_load_mime_types), 1);
        rb_UTF8  = rb_str_new2("UTF-8");
        rb_ASCII = rb_str_new2("US-ASCII");
    }
}
