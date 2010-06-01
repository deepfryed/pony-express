#include <ruby.h>
#include <ruby/encoding.h>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <exception>
#include <mimetic/mimetic.h>

#define ID_CONST_GET rb_intern("const_get")
#define CONST_GET(scope, constant) (rb_funcall(scope, ID_CONST_GET, 1, rb_str_new2(constant)))

static VALUE rb_mMimetic;
static VALUE eRuntimeError;
static VALUE eArgumentError;

using namespace std;
using namespace mimetic;

#define VALUEFUNC(f) ((VALUE (*)(ANYARGS)) f)

VALUE rb_mimetic_build(VALUE self, VALUE options) {
    ostringstream output;
    int message_id = 1;

    VALUE text    = rb_hash_aref(options, ID2SYM(rb_intern("text")));
    VALUE html    = rb_hash_aref(options, ID2SYM(rb_intern("html")));
    VALUE to      = rb_hash_aref(options, ID2SYM(rb_intern("to")));
    VALUE from    = rb_hash_aref(options, ID2SYM(rb_intern("from")));
    VALUE subject = rb_hash_aref(options, ID2SYM(rb_intern("subject")));

    if (text    == Qnil) rb_raise(eArgumentError, "Mimetic.build called without :text");
    if (from    == Qnil) rb_raise(eArgumentError, "Mimetic.build called without :from");
    if (to      == Qnil) rb_raise(eArgumentError, "Mimetic.build called without :to");
    if (subject == Qnil) rb_raise(eArgumentError, "Mimetic.build called without :subject");

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
            text_part->header().messageid(message_id++);
            text_part->header().mimeVersion(v1);
            text_part->body().assign(RSTRING_PTR(text));
            html_part->header().contentType("text/html; charset=UTF-8");
            html_part->header().contentTransferEncoding("7bit");
            html_part->header().messageid(message_id++);
            html_part->header().mimeVersion(v1);
            html_part->body().assign(RSTRING_PTR(html));
        }
        else {
            message->body().assign(RSTRING_PTR(text));
            message->header().contentType("text/plain; charset=UTF-8");
            message->header().contentTransferEncoding("8bit");
            message->header().messageid(message_id++);
            message->header().mimeVersion(v1);
        }
        
        message->header().from(RSTRING_PTR(from));
        message->header().to(RSTRING_PTR(to));
        message->header().subject(RSTRING_PTR(subject));
    
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
    }
}
