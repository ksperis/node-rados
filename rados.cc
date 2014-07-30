//#define BUILDING_NODE_EXTENSION
#include <node.h>
#include <node_buffer.h>
#include "rados.h"
#include <stdlib.h>
#include </usr/include/rados/librados.h>

using namespace v8;
using namespace node;

Persistent<Function> Rados::constructor;
Rados::Rados() {};
Rados::~Rados() {};

Persistent<Function> Ioctx::constructor;
Ioctx::Ioctx() {};
Ioctx::~Ioctx() {};


char *get(v8::Local<v8::Value> value, const char *fallback = "") {
    if (value->IsString()) {
        v8::String::AsciiValue string(value);
        char *str = (char *) malloc(string.length() + 1);
        strcpy(str, *string);
        return str;
    }
    char *str = (char *) malloc(strlen(fallback) + 1);
    strcpy(str, fallback);
    return str;
}


void Rados::Init(Handle<Object> target) {
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("Rados"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->PrototypeTemplate()->Set(String::NewSymbol("connect"),
      FunctionTemplate::New(connect)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("shutdown"),
      FunctionTemplate::New(connect)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("get_fsid"),
      FunctionTemplate::New(get_fsid)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("pool_list"),
      FunctionTemplate::New(pool_list)->GetFunction());

  constructor = Persistent<Function>::New(tpl->GetFunction());
  target->Set(String::NewSymbol("Rados"), constructor);
}


void Ioctx::Init(Handle<Object> target) {
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("Ioctx"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->PrototypeTemplate()->Set(String::NewSymbol("destroy"),
      FunctionTemplate::New(destroy)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("read"),
      FunctionTemplate::New(read)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("write_full"),
      FunctionTemplate::New(write_full)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("stat"),
      FunctionTemplate::New(stat)->GetFunction());

  constructor = Persistent<Function>::New(tpl->GetFunction());
  target->Set(String::NewSymbol("Ioctx"), constructor);
}


Handle<Value> Rados::New(const Arguments& args) {
  HandleScope scope;

  Rados* obj = new Rados();

  char* cluster_name = get(args[0],"ceph");
  char* user_name = get(args[1],"client.admin");
  char* conffile = get(args[2],"/etc/ceph/ceph.conf");
  uint64_t flags = 0;

  if ( rados_create2(&obj->cluster, cluster_name, user_name, flags) != 0 ) {
    return ThrowException(Exception::Error(String::New("create rados cluster failed")));
  }
  if ( rados_conf_read_file(obj->cluster, conffile) != 0 ) {
    return ThrowException(Exception::Error(String::New("read conffile failed")));
  }

  obj->Wrap(args.This());

  return args.This();
}


Handle<Value> Ioctx::New(const Arguments& args) {
  HandleScope scope;

  if (args.IsConstructCall()) {
    Ioctx* obj = new Ioctx();
    Rados* cluster = ObjectWrap::Unwrap<Rados>(
        args[0]->ToObject());
    char* pool = get(args[1], "");
    if ( rados_ioctx_create(cluster->cluster, pool, &obj->ioctx) != 0 ) {
      return ThrowException(Exception::Error(String::New("create Ioctx failed")));
    }
    obj->Wrap(args.This());
    return args.This();
  } else {
    const int argc = 1;
    Local<Value> argv[argc] = { args[0] };
    return scope.Close(constructor->NewInstance(argc, argv));
  }
}


Handle<Value> Rados::connect(const Arguments& args) {
  HandleScope scope;

  Rados* obj = ObjectWrap::Unwrap<Rados>(args.This());

  rados_connect(obj->cluster);

  return scope.Close(Null());
}


Handle<Value> Rados::shutdown(const Arguments& args) {
  HandleScope scope;

  Rados* obj = ObjectWrap::Unwrap<Rados>(args.This());

  rados_shutdown(obj->cluster);

  return scope.Close(Null());
}


Handle<Value> Rados::get_fsid(const Arguments& args) {
  HandleScope scope;

  Rados* obj = ObjectWrap::Unwrap<Rados>(args.This());
  
  char fsid[37];
  if ( rados_cluster_fsid(obj->cluster, fsid, sizeof(fsid)) < 0) {
    scope.Close(Null());
  }

  return scope.Close(String::New(fsid));
}


Handle<Value> Rados::pool_list(const Arguments& args) {
  HandleScope scope;

  Rados* obj = ObjectWrap::Unwrap<Rados>(args.This());
  
  char temp_buffer[256];
  int buff_size = rados_pool_list(obj->cluster, temp_buffer, 0);

  char buffer[buff_size];
  int r = rados_pool_list(obj->cluster, buffer, sizeof(buffer));
  if (r != buff_size) {
    scope.Close(Null());
  }

  return scope.Close(Buffer::New(buffer, buff_size)->handle_);
}


Handle<Value> Ioctx::destroy(const Arguments& args) {
  HandleScope scope;

  Ioctx* obj = ObjectWrap::Unwrap<Ioctx>(args.This());

  rados_ioctx_destroy(obj->ioctx);

  return scope.Close(Null());
}

Handle<Value> Ioctx::read(const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 2 ||
      !args[0]->IsString() ||
      !args[1]->IsNumber()) {
    return ThrowException(Exception::Error(String::New("Bad argument.")));
  }

  Ioctx* obj = ObjectWrap::Unwrap<Ioctx>(args.This());
  char* oid = get(args[0], "");
  size_t size = args[1]->Uint32Value();
  uint64_t offset = args[2]->IsNumber() ? args[2]->IntegerValue() : 1;

  char buffer[size];

  int err = rados_read(obj->ioctx, oid, buffer, size, offset);

  if (err < 0) {
    return scope.Close(Null());
  }

  return scope.Close(Buffer::New(buffer, size)->handle_);
}


Handle<Value> Ioctx::write_full(const Arguments& args) {
  HandleScope scope;

  if (args.Length() != 2 ||
      !args[0]->IsString() ||
      !Buffer::HasInstance(args[1])) {
    return ThrowException(Exception::Error(String::New("Bad argument.")));
  }

  Ioctx* obj = ObjectWrap::Unwrap<Ioctx>(args.This());
  char* oid = get(args[0], "");
  char* buffer = Buffer::Data(args[1]);

  int err = rados_write_full(obj->ioctx, oid, buffer, sizeof(buffer));

  if (err < 0) {
    return scope.Close(Null());
  }

  return scope.Close(Number::New(err));
}


Handle<Value> Ioctx::stat(const Arguments& args) {
  HandleScope scope;

  if (args.Length() != 1 ||
      !args[0]->IsString()) {
    return scope.Close(Null());
  }

  Ioctx* obj = ObjectWrap::Unwrap<Ioctx>(args.This());
  char *oid = get(args[0], "");
  uint64_t psize;
  time_t pmtime;

  int err = rados_stat(obj->ioctx, oid, &psize, &pmtime);
  if (err < 0) {
    return scope.Close(Null());
  }

  Local<Object> stat = Object::New( );
  stat->Set( String::NewSymbol("oid"), String::New(oid) );
  stat->Set( String::NewSymbol("psize"), Number::New(psize) );
  stat->Set( String::NewSymbol("pmtime"), Number::New(pmtime) );

  return scope.Close(stat);
}


