#include <ruby.h>
#include <ruby/encoding.h>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <exception>
#include <algorithm>
#include <pcre++.h>
#include <mimetic/mimetic.h>

#define ID_CONST_GET rb_intern("const_get")
#define CONST_GET(scope, constant) (rb_funcall(scope, ID_CONST_GET, 1, rb_str_new2(constant)))

static VALUE rb_mMimetic;
static VALUE eRuntimeError;
static VALUE eArgumentError;

using namespace std;
using namespace mimetic;
using namespace pcrepp;

#define VALUEFUNC(f) ((VALUE (*)(ANYARGS)) f)

map<string, string> MimeTypes;

/* RFC References

http://www.ietf.org/rfc/rfc2822.txt  Internet Message Format.
http://www.ietf.org/rfc/rfc2045.txt  MIME Extensions - Multipart related (Part One).
http://www.ietf.org/rfc/rfc2046.txt  MIME Extensions - Multipart related (Part Deux).
http://www.ietf.org/rfc/rfc2392.txt  Content-ID and Message-ID.

*/

string file_mime_type(string file) {
    Pcre regex("\\.");
    string extn = regex.split(file).back();
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

void rb_load_mime_types(VALUE self, VALUE filename) {
    char buffer[4096];
    vector<string> data;
    Pcre regex("[\\r\\n\\t]+");
    ifstream file(RSTRING_PTR(filename), ios::in);
    if (file.is_open()) {
        while (!file.eof()) {
            file.getline(buffer, 4096);
            data = regex.split(buffer);
            for (int i = 1; i < data.size(); i++)
                MimeTypes[data[i]] = data[0];
        }
        file.close();
    }
    else {
        rb_raise(eRuntimeError, "Mimetic: Unable to load mime.types");
    }
}

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
    VALUE html    = rb_hash_aref(options, ID2SYM(rb_intern("html")));
    VALUE tcid    = rb_hash_aref(options, ID2SYM(rb_intern("text_content_id")));
    VALUE hcid    = rb_hash_aref(options, ID2SYM(rb_intern("html_content_id")));
    VALUE replyto = rb_hash_aref(options, ID2SYM(rb_intern("replyto")));
    VALUE cc      = rb_hash_aref(options, ID2SYM(rb_intern("cc")));
    VALUE bcc     = rb_hash_aref(options, ID2SYM(rb_intern("bcc")));
    VALUE files   = rb_hash_aref(options, ID2SYM(rb_intern("attachments")));

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
            text_part->header().contentType("text/plain; charset=UTF-8");
            text_part->header().contentTransferEncoding("8bit");
            text_part->header().contentId(tcid == Qnil ? content_id() + ">" : ContentId(RSTRING_PTR(tcid)));
            text_part->header().mimeVersion(v1);
            text_part->body().assign(RSTRING_PTR(text));
            html_part->header().contentType("text/html; charset=UTF-8");
            html_part->header().contentTransferEncoding("7bit");
            html_part->header().contentId(hcid == Qnil ? content_id() + ">" : ContentId(RSTRING_PTR(hcid)));
            html_part->header().mimeVersion(v1);
            html_part->body().assign(RSTRING_PTR(html));
        }
        else {
            message->body().assign(RSTRING_PTR(text));
            message->header().contentType("text/plain; charset=UTF-8");
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
                    mimetic_attach_file(message, RSTRING_PTR(attachment));
            }
        }

        message->header().from(RSTRING_PTR(from));
        message->header().to(RSTRING_PTR(to));
        message->header().subject(RSTRING_PTR(subject));
        message->header().messageid(message_id(1));

        if (replyto != Qnil) message->header().replyto(RSTRING_PTR(replyto));
        if (cc      != Qnil) message->header().cc(RSTRING_PTR(cc));
        if (bcc     != Qnil) message->header().bcc(RSTRING_PTR(bcc));

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

    rb_raise(eRuntimeError, "Mimetic boo boo : %s\n", RSTRING_PTR(errors));
}

extern "C"  {
    void Init_mimetic(void) {
        eRuntimeError  = CONST_GET(rb_mKernel, "RuntimeError");
        eArgumentError = CONST_GET(rb_mKernel, "ArgumentError");
        rb_mMimetic = rb_define_module("Mimetic");
        rb_define_module_function(rb_mMimetic, "build", VALUEFUNC(rb_mimetic_build), 1);
        rb_define_module_function(rb_mMimetic, "load_mime_types", VALUEFUNC(rb_load_mime_types), 1);
    }
}
