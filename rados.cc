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


void Rados::Init(Handle<Object> target) {
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("Rados"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->PrototypeTemplate()->Set(String::NewSymbol("connect"),
      FunctionTemplate::New(connect)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("shutdown"),
      FunctionTemplate::New(shutdown)->GetFunction());
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
  tpl->PrototypeTemplate()->Set(String::NewSymbol("snap_create"),
      FunctionTemplate::New(snap_create)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("snap_remove"),
      FunctionTemplate::New(snap_remove)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("read"),
      FunctionTemplate::New(read)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("write"),
      FunctionTemplate::New(write)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("write_full"),
      FunctionTemplate::New(write_full)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("clone_range"),
      FunctionTemplate::New(clone_range)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("append"),
      FunctionTemplate::New(append)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("remove"),
      FunctionTemplate::New(remove)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("trunc"),
      FunctionTemplate::New(trunc)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("stat"),
      FunctionTemplate::New(stat)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("getxattr"),
      FunctionTemplate::New(getxattr)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("setxattr"),
      FunctionTemplate::New(setxattr)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("rmxattr"),
      FunctionTemplate::New(rmxattr)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("getxattrs"),
      FunctionTemplate::New(getxattrs)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("aio_read"),
      FunctionTemplate::New(aio_read)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("aio_write"),
      FunctionTemplate::New(aio_write)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("aio_flush"),
      FunctionTemplate::New(aio_flush)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("aio_flush_async"),
      FunctionTemplate::New(aio_flush_async)->GetFunction());

  constructor = Persistent<Function>::New(tpl->GetFunction());
  target->Set(String::NewSymbol("Ioctx"), constructor);
}


Handle<Value> Rados::New(const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 3 ||
      !args[0]->IsString() ||
      !args[1]->IsString() ||
      !args[2]->IsString()) {
    return ThrowException(Exception::Error(String::New("Bad argument.")));
  }

  Rados* obj = new Rados();
  String::Utf8Value cluster_name(args[0]);
  String::Utf8Value user_name(args[1]);
  String::Utf8Value conffile(args[2]);
  uint64_t flags = 0;

  if ( rados_create2(&obj->cluster, *cluster_name, *user_name, flags) != 0 ) {
    return ThrowException(Exception::Error(String::New("create rados cluster failed")));
  }
  if ( rados_conf_read_file(obj->cluster, *conffile) != 0 ) {
    return ThrowException(Exception::Error(String::New("read conffile failed")));
  }

  obj->Wrap(args.This());
  return args.This();
}


Handle<Value> Ioctx::New(const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 2 ||
      !args[1]->IsString()) {
    return ThrowException(Exception::Error(String::New("Bad argument.")));
  }

  Ioctx* obj = new Ioctx();
  Rados* cluster = ObjectWrap::Unwrap<Rados>(args[0]->ToObject());
  String::Utf8Value pool(args[1]);
  if ( rados_ioctx_create(cluster->cluster, *pool, &obj->ioctx) != 0 ) {
    return ThrowException(Exception::Error(String::New("create Ioctx failed")));
  }

  obj->Wrap(args.This());
  return args.This();
}


Handle<Value> Rados::connect(const Arguments& args) {
  HandleScope scope;

  Rados* obj = ObjectWrap::Unwrap<Rados>(args.This());

  int err = rados_connect(obj->cluster);

  return scope.Close(Number::New(-err));
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

  Local<Array> pools = Array::New();
  const char *b = buffer;
  uint32_t array_id = 0;
  while (1) {
      if (b[array_id] == '\0') {
          break;
      }
      pools->Set(array_id, String::New(b));
      b += strlen(b) + 1;
      array_id++;
  }

  return scope.Close(pools);
}


Handle<Value> Ioctx::destroy(const Arguments& args) {
  HandleScope scope;

  Ioctx* obj = ObjectWrap::Unwrap<Ioctx>(args.This());

  rados_ioctx_destroy(obj->ioctx);

  return scope.Close(Null());
}


Handle<Value> Ioctx::snap_create(const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 1 ||
      !args[0]->IsString()) {
    return ThrowException(Exception::Error(String::New("Bad argument.")));
  }

  Ioctx* obj = ObjectWrap::Unwrap<Ioctx>(args.This());
  String::Utf8Value snapname(args[0]);

  int err = rados_ioctx_snap_create(obj->ioctx, *snapname);

  return scope.Close(Number::New(-err));
}


Handle<Value> Ioctx::snap_remove(const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 1 ||
      !args[0]->IsString()) {
    return ThrowException(Exception::Error(String::New("Bad argument.")));
  }

  Ioctx* obj = ObjectWrap::Unwrap<Ioctx>(args.This());
  String::Utf8Value snapname(args[0]);

  int err = rados_ioctx_snap_remove(obj->ioctx, *snapname);

  return scope.Close(Number::New(-err));
}


Handle<Value> Ioctx::read(const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 1 ||
      !args[0]->IsString()) {
    return ThrowException(Exception::Error(String::New("Bad argument.")));
  }

  Ioctx* obj = ObjectWrap::Unwrap<Ioctx>(args.This());
  String::Utf8Value oid(args[0]);
  size_t size;
  if (args[3]->IsNumber()) {
    size = args[3]->Uint32Value();
  } else {
    if ( rados_stat(obj->ioctx, *oid, &size, NULL) < 0) {
      return scope.Close(Null());
    }
  }
  uint64_t offset = args[2]->IsNumber() ? args[2]->IntegerValue() : 0;

  char buffer[size];

  int err = rados_read(obj->ioctx, *oid, buffer, size, offset);

  if (err < 0) {
    return scope.Close(Null());
  }

  return scope.Close(Buffer::New(buffer, size)->handle_);
}


Handle<Value> Ioctx::write(const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 2 ||
      !args[0]->IsString() ||
      !Buffer::HasInstance(args[1])) {
    return ThrowException(Exception::Error(String::New("Bad argument.")));
  }

  Ioctx* obj = ObjectWrap::Unwrap<Ioctx>(args.This());
  String::Utf8Value oid(args[0]);
  char* buffer = Buffer::Data(args[1]);
  size_t size = args[2]->IsNumber() ? args[2]->Uint32Value() : strlen(buffer);
  uint64_t offset = args[3]->IsNumber() ? args[3]->IntegerValue() : 0;

  int err = rados_write(obj->ioctx, *oid, buffer, size, offset);

  return scope.Close(Number::New(-err));
}


Handle<Value> Ioctx::write_full(const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 2 ||
      !args[0]->IsString() ||
      !Buffer::HasInstance(args[1])) {
    return ThrowException(Exception::Error(String::New("Bad argument.")));
  }

  Ioctx* obj = ObjectWrap::Unwrap<Ioctx>(args.This());
  String::Utf8Value oid(args[0]);
  char* buffer = Buffer::Data(args[1]);
  size_t size = args[2]->IsNumber() ? args[2]->Uint32Value() : strlen(buffer);

  int err = rados_write_full(obj->ioctx, *oid, buffer, size);

  return scope.Close(Number::New(-err));
}


Handle<Value> Ioctx::clone_range(const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 5 ||
      !args[0]->IsString() ||
      !args[1]->IsNumber() ||
      !args[2]->IsNumber() ||
      !args[3]->IsNumber() ||
      !args[4]->IsNumber()) {
    return ThrowException(Exception::Error(String::New("Bad argument.")));
  }

  Ioctx* obj = ObjectWrap::Unwrap<Ioctx>(args.This());
  String::Utf8Value dst(args[0]);
  uint64_t dst_off = args[1]->Uint32Value();
  String::Utf8Value src(args[0]);
  uint64_t src_off = args[3]->Uint32Value();
  size_t size = args[2]->Uint32Value();

  int err = rados_clone_range(obj->ioctx, *dst, dst_off, *src, src_off, size);

  return scope.Close(Number::New(-err));
}


Handle<Value> Ioctx::append(const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 2 ||
      !args[0]->IsString() ||
      !Buffer::HasInstance(args[1])) {
    return ThrowException(Exception::Error(String::New("Bad argument.")));
  }

  Ioctx* obj = ObjectWrap::Unwrap<Ioctx>(args.This());
  String::Utf8Value oid(args[0]);
  char* buffer = Buffer::Data(args[1]);
  size_t size = args[2]->IsNumber() ? args[2]->Uint32Value() : strlen(buffer);

  int err = rados_append(obj->ioctx, *oid, buffer, size);

  return scope.Close(Number::New(-err));
}


Handle<Value> Ioctx::remove(const Arguments& args) {
  HandleScope scope;

  if (args.Length() != 1 ||
      !args[0]->IsString()) {
    return scope.Close(Null());
  }

  Ioctx* obj = ObjectWrap::Unwrap<Ioctx>(args.This());
  String::Utf8Value oid(args[0]);

  int err = rados_remove(obj->ioctx, *oid);

  return scope.Close(Number::New(-err));
}


Handle<Value> Ioctx::trunc(const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 2 ||
      !args[0]->IsString() ||
      !args[1]->IsNumber()) {
    return ThrowException(Exception::Error(String::New("Bad argument.")));
  }

  Ioctx* obj = ObjectWrap::Unwrap<Ioctx>(args.This());
  String::Utf8Value oid(args[0]);
  size_t size = args[1]->Uint32Value();

  int err = rados_trunc(obj->ioctx, *oid, size);

  return scope.Close(Number::New(-err));
}


Handle<Value> Ioctx::getxattr(const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 2 ||
      !args[0]->IsString() ||
      !args[1]->IsString()) {
    return ThrowException(Exception::Error(String::New("Bad argument.")));
  }

  Ioctx* obj = ObjectWrap::Unwrap<Ioctx>(args.This());
  String::Utf8Value oid(args[0]);
  String::Utf8Value name(args[1]);
  size_t size;
  if (args[2]->IsNumber()) {
    size = args[2]->Uint32Value();
  } else {
    char temp_buffer[DEFAULT_BUFFER_SIZE];
    int ret = rados_getxattr(obj->ioctx, *oid, *name, temp_buffer, 0);
    if (ret < 0) {
      return scope.Close(Null());
    } else {
      size = ret;
    }
  }
  char buffer[size];

  int err = rados_getxattr(obj->ioctx, *oid, *name, buffer, size);

  if (err < 0) {
    return scope.Close(Null());
  }

  return scope.Close(String::New(buffer, size));
}


Handle<Value> Ioctx::setxattr(const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 3 ||
      !args[0]->IsString() ||
      !args[1]->IsString() ||
      !args[2]->IsString()) {
    return ThrowException(Exception::Error(String::New("Bad argument.")));
  }

  Ioctx* obj = ObjectWrap::Unwrap<Ioctx>(args.This());
  String::Utf8Value oid(args[0]);
  String::Utf8Value name(args[1]);
  String::Utf8Value buffer(args[2]);
  size_t size = args[3]->IsNumber() ? args[3]->Uint32Value() : strlen(*buffer);

  int err = rados_setxattr(obj->ioctx, *oid, *name, *buffer, size);

  return scope.Close(Number::New(-err));
}


Handle<Value> Ioctx::rmxattr(const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 2 ||
      !args[0]->IsString() ||
      !args[1]->IsString()) {
    return ThrowException(Exception::Error(String::New("Bad argument.")));
  }

  Ioctx* obj = ObjectWrap::Unwrap<Ioctx>(args.This());
  String::Utf8Value oid(args[0]);
  String::Utf8Value name(args[1]);

  int err = rados_rmxattr(obj->ioctx, *oid, *name);

  return scope.Close(Number::New(-err));
}


Handle<Value> Ioctx::getxattrs(const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 1 ||
      !args[0]->IsString()) {
    return ThrowException(Exception::Error(String::New("Bad argument.")));
  }

  Ioctx* obj = ObjectWrap::Unwrap<Ioctx>(args.This());
  String::Utf8Value oid(args[0]);
  rados_xattrs_iter_t iter;

  Local<Object> xattrs = Object::New();
  int err = rados_getxattrs(obj->ioctx, *oid, &iter);
  if (err < 0) {
    return scope.Close(Null());
  }
  while (1) {
      const char *name;
      const char *val;
      size_t len;

      err = rados_getxattrs_next(iter, &name, &val, &len);
      if (err < 0) {
        return scope.Close(Null());
      }
      if (name == NULL) {
          break;
      }

      xattrs->Set(String::NewSymbol(name), String::New(val, len));
  }
  rados_getxattrs_end(iter);

  return scope.Close(xattrs);
}


Handle<Value> Ioctx::stat(const Arguments& args) {
  HandleScope scope;

  if (args.Length() != 1 ||
      !args[0]->IsString()) {
    return scope.Close(Null());
  }

  Ioctx* obj = ObjectWrap::Unwrap<Ioctx>(args.This());
  String::Utf8Value oid(args[0]);
  uint64_t psize;
  time_t pmtime;

  int err = rados_stat(obj->ioctx, *oid, &psize, &pmtime);
  if (err < 0) {
    return scope.Close(Null());
  }

  Local<Object> stat = Object::New( );
  stat->Set( String::NewSymbol("oid"), String::New(*oid) );
  stat->Set( String::NewSymbol("psize"), Number::New(psize) );
  stat->Set( String::NewSymbol("pmtime"), Number::New(pmtime) );

  return scope.Close(stat);
}


void Ioctx::wait_complete(uv_work_t *req) {
  AsyncData* asyncdata = (AsyncData *)req->data;

  int err = rados_aio_wait_for_complete(*asyncdata->comp);
  if (err < 0) {
    asyncdata->err = -err;
  }

  rados_aio_release(*asyncdata->comp);
  delete asyncdata->comp;
}


void Ioctx::callback_complete(uv_work_t *req) {
  HandleScope scope;

  AsyncData *asyncdata = (AsyncData *)req->data;

  if (asyncdata->cb_buffer) {
    const unsigned argc = 2;
    Local<Value> argv[argc] = {
      Local<Value>::New(Number::New(asyncdata->err)),
      Local<Value>::New(Buffer::New(asyncdata->buffer, asyncdata->size)->handle_) };
    asyncdata->callback->Call(Context::GetCurrent()->Global(), argc, argv);
  }
  else {
    const unsigned argc = 1;
    Local<Value> argv[argc] = {
      Local<Value>::New(Number::New(asyncdata->err)) };
    asyncdata->callback->Call(Context::GetCurrent()->Global(), argc, argv);
  }
  
  delete asyncdata;
}


Handle<Value> Ioctx::aio_read(const Arguments& args) {
  HandleScope scope;

  if (args.Length() != 4 ||
      !args[0]->IsString() ||
      !args[1]->IsNumber() ||
      !args[2]->IsNumber() ||
      !args[3]->IsFunction()) {
    return ThrowException(Exception::Error(String::New("Bad argument.")));
  }

  Ioctx* obj = ObjectWrap::Unwrap<Ioctx>(args.This());
  String::Utf8Value oid(args[0]);
  size_t size = args[1]->Uint32Value();
  uint64_t offset = args[2]->IsNumber() ? args[2]->IntegerValue() : 1;
  Persistent<Function> cb_complete = Persistent<Function>::New(Local<Function>::Cast(args[3]));

  AsyncData *asyncdata = new AsyncData;
  char *buffer = new char[size];
  rados_completion_t *comp = new rados_completion_t;

  asyncdata->callback = cb_complete;
  asyncdata->buffer = buffer;
  asyncdata->cb_buffer = true;
  asyncdata->size = size;
  asyncdata->comp = comp;
  asyncdata->err = 0;

  rados_aio_create_completion(NULL, NULL, NULL, comp);
  int err = rados_aio_read(obj->ioctx, *oid, *comp, buffer, size, offset);
  if (err < 0) {
    asyncdata->err = -err;
  }

  uv_work_t *req = new uv_work_t;
  req->data = asyncdata;

  uv_queue_work(
    uv_default_loop(),
    req,
    (uv_work_cb)wait_complete,
    (uv_after_work_cb)callback_complete
  );

  return scope.Close(Undefined());
}


Handle<Value> Ioctx::aio_write(const Arguments& args) {
  HandleScope scope;

  if (args.Length() != 5 ||
      !args[0]->IsString() ||
      !Buffer::HasInstance(args[1]) ||
      !args[2]->IsNumber() ||
      !args[3]->IsNumber() ||
      !args[4]->IsFunction()) {
    return ThrowException(Exception::Error(String::New("Bad argument.")));
  }

  Ioctx* obj = ObjectWrap::Unwrap<Ioctx>(args.This());
  String::Utf8Value oid(args[0]);
  char* buffer = Buffer::Data(args[1]);
  size_t size = args[2]->Uint32Value();
  uint64_t offset = args[3]->Uint32Value();
  Persistent<Function> cb_complete = Persistent<Function>::New(Local<Function>::Cast(args[4]));

  AsyncData *asyncdata = new AsyncData;
  rados_completion_t *comp = new rados_completion_t;

  asyncdata->callback = cb_complete;
  asyncdata->buffer = buffer;
  asyncdata->cb_buffer = false;
  asyncdata->size = size;
  asyncdata->comp = comp;
  asyncdata->err = 0;

  rados_aio_create_completion(NULL, NULL, NULL, comp);
  int err = rados_aio_write(obj->ioctx, *oid, *comp, buffer, size, offset);
  if (err < 0) {
    asyncdata->err = -err;
  }

  uv_work_t *req = new uv_work_t;
  req->data = asyncdata;

  uv_queue_work(
    uv_default_loop(),
    req,
    (uv_work_cb)wait_complete,
    (uv_after_work_cb)callback_complete
  );

  return scope.Close(Undefined());
}


Handle<Value> Ioctx::aio_flush(const Arguments& args) {
  HandleScope scope;

  Ioctx* obj = ObjectWrap::Unwrap<Ioctx>(args.This());
  
  int err = rados_aio_flush(obj->ioctx);

  return scope.Close(Number::New(-err));
}


Handle<Value> Ioctx::aio_flush_async(const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 1 ||
      !args[0]->IsFunction()) {
    return ThrowException(Exception::Error(String::New("Bad argument.")));
  }

  Ioctx* obj = ObjectWrap::Unwrap<Ioctx>(args.This());
  Persistent<Function> cb = Persistent<Function>::New(Local<Function>::Cast(args[1]));

  AsyncData *asyncdata = new AsyncData;
  rados_completion_t *comp = new rados_completion_t;

  asyncdata->callback = cb;
  asyncdata->cb_buffer = false;
  asyncdata->comp = comp;
  asyncdata->err = 0;

  rados_aio_create_completion(NULL, NULL, NULL, comp);
  int err = rados_aio_flush_async(obj->ioctx, *comp);
  if (err < 0) {
    asyncdata->err = -err;
  }

  uv_work_t *req = new uv_work_t;
  req->data = asyncdata;

  uv_queue_work(
    uv_default_loop(),
    req,
    (uv_work_cb)wait_complete,
    (uv_after_work_cb)callback_complete
  );

  return scope.Close(Undefined());
}