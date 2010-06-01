#include <ruby.h>
#include <ruby/encoding.h>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <mimetic/mimetic.h>

#define ID_CONST_GET rb_intern("const_get")
#define CONST_GET(scope, constant) (rb_funcall(scope, ID_CONST_GET, 1, rb_str_new2(constant)))

static VALUE rb_mMimetic;
static VALUE eLoadError;
static VALUE eRuntimeError;
static VALUE eArgumentError;

using namespace std;
using namespace mimetic;

#define VALUEFUNC(f) ((VALUE (*)(ANYARGS)) f)

VALUE rb_mimetic_build(VALUE self, VALUE options) {
    ostringstream output;

    VALUE text    = rb_hash_aref(options, ID2SYM(rb_intern("text")));
    VALUE html    = rb_hash_aref(options, ID2SYM(rb_intern("html")));
    VALUE to      = rb_hash_aref(options, ID2SYM(rb_intern("to")));
    VALUE from    = rb_hash_aref(options, ID2SYM(rb_intern("from")));
    VALUE subject = rb_hash_aref(options, ID2SYM(rb_intern("subject")));

    MimeEntity *message = new MimeEntity;
    if (html != Qnil && text != Qnil) {
        delete message;
        message = new MultipartAlternative;
        MimeEntity *html_part = new MimeEntity;
        MimeEntity *text_part = new MimeEntity;
        text_part->header().contentType("text/plain; charset=utf-8");
        text_part->body().assign(RSTRING_PTR(text));
        html_part->header().contentType("text/html; charset=utf-8");
        html_part->body().assign(RSTRING_PTR(html));
        message->body().parts().push_back(text_part);
        message->body().parts().push_back(html_part);
    }
    else {
        message->body().assign(RSTRING_PTR(text));
        message->header().contentType("text/plain; charset=utf-8");
    }
    
    message->header().from(RSTRING_PTR(from));
    message->header().to(RSTRING_PTR(to));
    message->header().subject(RSTRING_PTR(subject));

    output << *message << endl;
    delete message;
    return rb_str_new2(output.str().c_str());
}
    
    
extern "C"  {
    void Init_mimetic(void) {
        eLoadError     = CONST_GET(rb_mKernel, "LoadError");
        eRuntimeError  = CONST_GET(rb_mKernel, "RuntimeError");
        eArgumentError = CONST_GET(rb_mKernel, "ArgumentError");
        rb_mMimetic = rb_define_module("Mimetic");
        rb_define_module_function(rb_mMimetic, "build", VALUEFUNC(rb_mimetic_build), 1);
    }
}
