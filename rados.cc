//#define BUILDING_NODE_EXTENSION
#include <node.h>
#include <node_buffer.h>
#include "rados.h"
#include <stdlib.h>
#include </usr/include/rados/librados.h>

using namespace v8;
using namespace node;

Nan::Persistent<v8::FunctionTemplate> Rados::constructor;
Rados::Rados() {};
Rados::~Rados() {};

Nan::Persistent<v8::FunctionTemplate> Ioctx::constructor;
Ioctx::Ioctx() {};
Ioctx::~Ioctx() {};


void Rados::Init(Local<Object> target) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New<String>("Rados").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "connect", connect);
  Nan::SetPrototypeMethod(tpl, "shutdown", shutdown);
  Nan::SetPrototypeMethod(tpl, "get_fsid", get_fsid);
  Nan::SetPrototypeMethod(tpl, "pool_create", pool_create);
  Nan::SetPrototypeMethod(tpl, "pool_delete", pool_delete);
  Nan::SetPrototypeMethod(tpl, "pool_list", pool_list);

  constructor.Reset(tpl);
  target->Set(Nan::New<String>("Rados").ToLocalChecked(),
      Nan::New<FunctionTemplate>(constructor)->GetFunction());
}


void Ioctx::Init(Local<Object> target) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New<String>("Ioctx").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "pool_set_auid", pool_set_auid);
  Nan::SetPrototypeMethod(tpl, "pool_get_auid", pool_get_auid);
  Nan::SetPrototypeMethod(tpl, "destroy", destroy);
  Nan::SetPrototypeMethod(tpl, "snap_create", snap_create);
  Nan::SetPrototypeMethod(tpl, "snap_remove", snap_remove);
  Nan::SetPrototypeMethod(tpl, "snap_rollback", snap_rollback);
  Nan::SetPrototypeMethod(tpl, "read", read);
  Nan::SetPrototypeMethod(tpl, "write", write);
  Nan::SetPrototypeMethod(tpl, "write_full", write_full);
  Nan::SetPrototypeMethod(tpl, "append", append);
  Nan::SetPrototypeMethod(tpl, "remove", remove);
  Nan::SetPrototypeMethod(tpl, "trunc", trunc);
  Nan::SetPrototypeMethod(tpl, "stat", stat);
  Nan::SetPrototypeMethod(tpl, "getxattr", getxattr);
  Nan::SetPrototypeMethod(tpl, "setxattr", setxattr);
  Nan::SetPrototypeMethod(tpl, "rmxattr", rmxattr);
  Nan::SetPrototypeMethod(tpl, "aio_read", aio_read);
  Nan::SetPrototypeMethod(tpl, "aio_append", aio_append);
  Nan::SetPrototypeMethod(tpl, "aio_write", aio_write);
  Nan::SetPrototypeMethod(tpl, "aio_write_full", aio_write_full);
  Nan::SetPrototypeMethod(tpl, "aio_flush", aio_flush);
  Nan::SetPrototypeMethod(tpl, "aio_flush_async", aio_flush_async);
  Nan::SetPrototypeMethod(tpl, "objects_list", objects_list);

  constructor.Reset(tpl);
  target->Set(Nan::New<String>("Ioctx").ToLocalChecked(),
      Nan::New<FunctionTemplate>(constructor)->GetFunction());
}

bool Rados::require_connected() {
  if ( this->state == STATE_CONNECTED ) {
    return true;
  } else {
    Nan::ThrowError("Cluster is not in connected state");
    return false;
  }
}

bool Ioctx::require_created() {
  if ( this->rados->require_connected() ) {
    if ( this->state == STATE_CREATED ) {
      return true;
    } else {
      Nan::ThrowError("Ioctx not in created state");
      return false;
    }
  } else {
    return false;
  }
}


NAN_METHOD(Rados::New) {
  if (!info.IsConstructCall()) {
    return Nan::ThrowError("Rados object must be instantiated with 'new' statement");
  }
  if (info.Length() < 3 ||
      !info[0]->IsString() ||
      !info[1]->IsString() ||
      !info[2]->IsString()) {
    return Nan::ThrowError("Bad argument.");
  }

  Rados* obj = new Rados();
  String::Utf8Value cluster_name(info[0]);
  String::Utf8Value user_name(info[1]);
  String::Utf8Value conffile(info[2]);
  uint64_t flags = 0;

  if ( rados_create2(&obj->cluster, *cluster_name, *user_name, flags) != 0 ) {
    return Nan::ThrowError("create rados cluster failed");
  }
  obj->state = STATE_CREATED;
  if ( rados_conf_read_file(obj->cluster, *conffile) != 0 ) {
    return Nan::ThrowError("read conffile failed");
  }
  obj->state = STATE_CONFIGURED;

  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}


NAN_METHOD(Ioctx::New) {
  if (!info.IsConstructCall()) {
    return Nan::ThrowError("Ioctx object must be instantiated with 'new' statement");
  }
  if (info.Length() < 2 ||
      !info[1]->IsString()) {
    return Nan::ThrowError("Bad argument.");
  }

  Ioctx* obj = new Ioctx();
  Rados* cluster = Nan::ObjectWrap::Unwrap<Rados>(info[0]->ToObject());
  if ( !cluster->require_connected() ) info.GetReturnValue().Set(Nan::Null());
  String::Utf8Value pool(info[1]);
  if ( rados_ioctx_create(cluster->cluster, *pool, &obj->ioctx) != 0 ) {
    return Nan::ThrowError("create Ioctx failed");
  }
  obj->rados = cluster;
  obj->state = STATE_CREATED;

  obj->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}


NAN_METHOD(Rados::connect) {
  Rados* obj = Nan::ObjectWrap::Unwrap<Rados>(info.This());

  if ( obj->state != STATE_CONFIGURED )
    return Nan::ThrowError("Cluster should be in configured state.");

  int err = rados_connect(obj->cluster);

  if (err == 0) {
    obj->state = STATE_CONNECTED;
  }

  info.GetReturnValue().Set(Nan::New<Number>(-err));
}


NAN_METHOD(Rados::shutdown) {
  Rados* obj = Nan::ObjectWrap::Unwrap<Rados>(info.This());
  if ( !obj->require_connected() ) info.GetReturnValue().Set(Nan::Null());

  rados_shutdown(obj->cluster);
  obj->state = STATE_DESTROYED;

  info.GetReturnValue().Set(Nan::Null());
}


NAN_METHOD(Rados::get_fsid) {
  Rados* obj = Nan::ObjectWrap::Unwrap<Rados>(info.This());
  if ( !obj->require_connected() ) info.GetReturnValue().Set(Nan::Null());

  char fsid[37];
  if ( rados_cluster_fsid(obj->cluster, fsid, sizeof(fsid)) < 0) {
    info.GetReturnValue().Set(Nan::Null());
  }

  info.GetReturnValue().Set(Nan::New<String>(fsid).ToLocalChecked());
}


NAN_METHOD(Rados::pool_create) {
  if (info.Length() < 1 ||
      !info[0]->IsString()) {
    return Nan::ThrowError("Bad argument.");
  }

  Rados* obj = Nan::ObjectWrap::Unwrap<Rados>(info.This());
  if ( !obj->require_connected() ) info.GetReturnValue().Set(Nan::Null());

  String::Utf8Value pool_name(info[0]);

  int err=0;
  switch (info.Length()) {
    case 1: {
      err = rados_pool_create(obj->cluster, *pool_name);
      break;
    }
    case 2: {
      if (info[1]->IsNumber()) {
        uint64_t auid = info[1]->IntegerValue();
        err = rados_pool_create_with_auid(obj->cluster, *pool_name, auid);
      } else {
        return Nan::ThrowError("Bad argument.");
      }
      break;
    }
    case 3: {
      if (info[1]->IsNumber() && info[2]->IsNumber()) {
        uint64_t auid = info[1]->IntegerValue();
        uint8_t crush_rule = info[2]->Uint32Value();
        err = rados_pool_create_with_all(obj->cluster, *pool_name, auid, crush_rule);
      } else if (info[2]->IsNumber()) {
        uint8_t crush_rule = info[2]->Uint32Value();
        err = rados_pool_create_with_crush_rule(obj->cluster, *pool_name, crush_rule);
      } else {
        return Nan::ThrowError("Bad argument.");
      }
      break;
    }
  }

  info.GetReturnValue().Set(Nan::New<Number>(-err));
}


NAN_METHOD(Rados::pool_delete) {
  if (info.Length() < 1 ||
      !info[0]->IsString()) {
    return Nan::ThrowError("Bad argument.");
  }

  Rados* obj = Nan::ObjectWrap::Unwrap<Rados>(info.This());
  if ( !obj->require_connected() ) info.GetReturnValue().Set(Nan::Null());
  String::Utf8Value pool_name(info[0]);

  int err = rados_pool_delete(obj->cluster, *pool_name);

  info.GetReturnValue().Set(Nan::New<Number>(-err));
}


NAN_METHOD(Rados::pool_list) {
  Rados* obj = Nan::ObjectWrap::Unwrap<Rados>(info.This());
  if ( !obj->require_connected() ) info.GetReturnValue().Set(Nan::Null());

  char temp_buffer[256];
  int buff_size = rados_pool_list(obj->cluster, temp_buffer, 0);

  char buffer[buff_size];
  int r = rados_pool_list(obj->cluster, buffer, sizeof(buffer));
  if (r != buff_size) {
    info.GetReturnValue().Set(Nan::Null());
  }

  Local<Array> pools = Nan::New<Array>();
  const char *b = buffer;
  uint32_t array_id = 0;
  while (1) {
      if (*b == '\0') {
          break;
      }
      pools->Set(array_id, Nan::New<String>(b).ToLocalChecked());
      b += strlen(b) + 1;
      array_id++;
  }

  info.GetReturnValue().Set(pools);
}


NAN_METHOD(Ioctx::pool_set_auid) {
  if (info.Length() < 1 ||
      info[1]->IsNumber()) {
    return Nan::ThrowError("Bad argument.");
  }

  Ioctx* obj = Nan::ObjectWrap::Unwrap<Ioctx>(info.This());
  if ( !obj->require_created() ) info.GetReturnValue().Set(Nan::Null());
  uint64_t auid = info[0]->IntegerValue();

  int err = rados_ioctx_pool_set_auid(obj->ioctx, auid);

  info.GetReturnValue().Set(Nan::New<Number>(-err));
}


NAN_METHOD(Ioctx::pool_get_auid) {
  Ioctx* obj = Nan::ObjectWrap::Unwrap<Ioctx>(info.This());
  if ( !obj->require_created() ) info.GetReturnValue().Set(Nan::Null());

  uint64_t auid;
  int err = rados_ioctx_pool_get_auid(obj->ioctx, &auid);

  if (err < 0) {
	  info.GetReturnValue().Set(Nan::New<Number>(err));
  }

  info.GetReturnValue().Set(Nan::New<Number>(auid));
}


NAN_METHOD(Ioctx::destroy) {
  Ioctx* obj = Nan::ObjectWrap::Unwrap<Ioctx>(info.This());
  if ( !obj->require_created() ) info.GetReturnValue().Set(Nan::Null());

  rados_ioctx_destroy(obj->ioctx);
  obj->state = STATE_DESTROYED;

  info.GetReturnValue().Set(Nan::Null());
}


NAN_METHOD(Ioctx::snap_create) {
  if (info.Length() < 1 ||
      !info[0]->IsString()) {
    return Nan::ThrowError("Bad argument.");
  }

  Ioctx* obj = Nan::ObjectWrap::Unwrap<Ioctx>(info.This());
  if ( !obj->require_created() ) info.GetReturnValue().Set(Nan::Null());
  String::Utf8Value snapname(info[0]);

  int err = rados_ioctx_snap_create(obj->ioctx, *snapname);

  info.GetReturnValue().Set(Nan::New<Number>(-err));
}


NAN_METHOD(Ioctx::snap_remove) {
  if (info.Length() < 1 ||
      !info[0]->IsString()) {
    return Nan::ThrowError("Bad argument.");
  }

  Ioctx* obj = Nan::ObjectWrap::Unwrap<Ioctx>(info.This());
  if ( !obj->require_created() ) info.GetReturnValue().Set(Nan::Null());
  String::Utf8Value snapname(info[0]);

  int err = rados_ioctx_snap_remove(obj->ioctx, *snapname);

  info.GetReturnValue().Set(Nan::New<Number>(-err));
}


NAN_METHOD(Ioctx::snap_rollback) {
  if (info.Length() < 2 ||
      !info[0]->IsString() ||
      !info[0]->IsString()) {
    return Nan::ThrowError("Bad argument.");
  }

  Ioctx* obj = Nan::ObjectWrap::Unwrap<Ioctx>(info.This());
  if ( !obj->require_created() ) info.GetReturnValue().Set(Nan::Null());
  String::Utf8Value oid(info[0]);
  String::Utf8Value snapname(info[1]);

  int err = rados_ioctx_snap_rollback(obj->ioctx, *oid, *snapname);

  info.GetReturnValue().Set(Nan::New<Number>(-err));
}


NAN_METHOD(Ioctx::read) {
  if (info.Length() < 1 ||
      !info[0]->IsString()) {
    return Nan::ThrowError("Bad argument.");
  }

  Ioctx* obj = Nan::ObjectWrap::Unwrap<Ioctx>(info.This());
  if ( !obj->require_created() ) info.GetReturnValue().Set(Nan::Null());
  String::Utf8Value oid(info[0]);
  size_t size = info[1]->IsNumber() ? info[1]->IntegerValue() : 8192;
  uint64_t offset = info[2]->IsNumber() ? info[2]->IntegerValue() : 0;

  char *buffer = new char[size];

  int err = rados_read(obj->ioctx, *oid, buffer, size, offset);

  if (err < 0) {
    delete buffer;
    info.GetReturnValue().Set(Nan::Null());
  } else {
	  info.GetReturnValue().Set(Nan::NewBuffer(buffer, err).ToLocalChecked());
  }

}


NAN_METHOD(Ioctx::write) {
  if (info.Length() < 2 ||
      !info[0]->IsString() ||
      !Buffer::HasInstance(info[1])) {
    return Nan::ThrowError("Bad argument.");
  }

  Ioctx* obj = Nan::ObjectWrap::Unwrap<Ioctx>(info.This());
  if ( !obj->require_created() ) info.GetReturnValue().Set(Nan::Null());
  String::Utf8Value oid(info[0]);
  char* buffer = Buffer::Data(info[1]);
  size_t size = info[2]->IsNumber() ? info[2]->Uint32Value() : Buffer::Length(info[1]);
  uint64_t offset = info[3]->IsNumber() ? info[3]->IntegerValue() : 0;

  int err = rados_write(obj->ioctx, *oid, buffer, size, offset);

  info.GetReturnValue().Set(Nan::New<Number>(-err));
}


NAN_METHOD(Ioctx::write_full) {
  if (info.Length() < 2 ||
      !info[0]->IsString() ||
      !Buffer::HasInstance(info[1])) {
    return Nan::ThrowError("Bad argument.");
  }

  Ioctx* obj = Nan::ObjectWrap::Unwrap<Ioctx>(info.This());
  if ( !obj->require_created() ) info.GetReturnValue().Set(Nan::Null());
  String::Utf8Value oid(info[0]);
  char* buffer = Buffer::Data(info[1]);
  size_t size = info[2]->IsNumber() ? info[2]->Uint32Value() : Buffer::Length(info[1]);

  int err = rados_write_full(obj->ioctx, *oid, buffer, size);

  info.GetReturnValue().Set(Nan::New<Number>(-err));
}


NAN_METHOD(Ioctx::clone_range) {
  if (info.Length() < 5 ||
      !info[0]->IsString() ||
      !info[1]->IsNumber() ||
      !info[2]->IsString() ||
      !info[3]->IsNumber() ||
      !info[4]->IsNumber()) {
    return Nan::ThrowError("Bad argument.");
  }

  Ioctx* obj = Nan::ObjectWrap::Unwrap<Ioctx>(info.This());
  if ( !obj->require_created() ) info.GetReturnValue().Set(Nan::Null());
  String::Utf8Value dst(info[0]);
  uint64_t dst_off = info[1]->Uint32Value();
  String::Utf8Value src(info[2]);
  uint64_t src_off = info[3]->Uint32Value();
  size_t size = info[4]->Uint32Value();

  int err = rados_clone_range(obj->ioctx, *dst, dst_off, *src, src_off, size);

  info.GetReturnValue().Set(Nan::New<Number>(-err));
}


NAN_METHOD(Ioctx::append) {
  if (info.Length() < 2 ||
      !info[0]->IsString() ||
      !Buffer::HasInstance(info[1])) {
    return Nan::ThrowError("Bad argument.");
  }

  Ioctx* obj = Nan::ObjectWrap::Unwrap<Ioctx>(info.This());
  if ( !obj->require_created() ) info.GetReturnValue().Set(Nan::Null());
  String::Utf8Value oid(info[0]);
  char* buffer = Buffer::Data(info[1]);
  size_t size = info[2]->IsNumber() ? info[2]->Uint32Value() : Buffer::Length(info[1]);

  int err = rados_append(obj->ioctx, *oid, buffer, size);

  info.GetReturnValue().Set(Nan::New<Number>(-err));
}


NAN_METHOD(Ioctx::remove) {
  if (info.Length() != 1 ||
      !info[0]->IsString()) {
    info.GetReturnValue().Set(Nan::Null());
  }

  Ioctx* obj = Nan::ObjectWrap::Unwrap<Ioctx>(info.This());
  if ( !obj->require_created() ) info.GetReturnValue().Set(Nan::Null());
  String::Utf8Value oid(info[0]);

  int err = rados_remove(obj->ioctx, *oid);

  info.GetReturnValue().Set(Nan::New<Number>(-err));
}


NAN_METHOD(Ioctx::trunc) {
  if (info.Length() < 2 ||
      !info[0]->IsString() ||
      !info[1]->IsNumber()) {
    return Nan::ThrowError("Bad argument.");
  }

  Ioctx* obj = Nan::ObjectWrap::Unwrap<Ioctx>(info.This());
  if ( !obj->require_created() ) info.GetReturnValue().Set(Nan::Null());
  String::Utf8Value oid(info[0]);
  size_t size = info[1]->Uint32Value();

  int err = rados_trunc(obj->ioctx, *oid, size);

  info.GetReturnValue().Set(Nan::New<Number>(-err));
}


NAN_METHOD(Ioctx::getxattr) {
  if (info.Length() < 2 ||
      !info[0]->IsString() ||
      !info[1]->IsString()) {
    return Nan::ThrowError("Bad argument.");
  }

  Ioctx* obj = Nan::ObjectWrap::Unwrap<Ioctx>(info.This());
  if ( !obj->require_created() ) info.GetReturnValue().Set(Nan::Null());
  String::Utf8Value oid(info[0]);
  String::Utf8Value name(info[1]);
  size_t size;
  if (info[2]->IsNumber()) {
    size = info[2]->Uint32Value();
  } else {
    char temp_buffer[DEFAULT_BUFFER_SIZE];
    int ret = rados_getxattr(obj->ioctx, *oid, *name, temp_buffer, 0);
    if (ret < 0) {
      info.GetReturnValue().Set(Nan::Null());
    } else {
      size = ret;
    }
  }
  char buffer[size];

  int err = rados_getxattr(obj->ioctx, *oid, *name, buffer, size);

  if (err < 0) {
    info.GetReturnValue().Set(Nan::Null());
  }

  info.GetReturnValue().Set(Nan::New<String>(buffer, size).ToLocalChecked());
}


NAN_METHOD(Ioctx::setxattr) {
  if (info.Length() < 3 ||
      !info[0]->IsString() ||
      !info[1]->IsString() ||
      !info[2]->IsString()) {
    return Nan::ThrowError("Bad argument.");
  }

  Ioctx* obj = Nan::ObjectWrap::Unwrap<Ioctx>(info.This());
  if ( !obj->require_created() ) info.GetReturnValue().Set(Nan::Null());
  String::Utf8Value oid(info[0]);
  String::Utf8Value name(info[1]);
  String::Utf8Value buffer(info[2]);
  size_t size = info[3]->IsNumber() ? info[3]->Uint32Value() : strlen(*buffer);

  int err = rados_setxattr(obj->ioctx, *oid, *name, *buffer, size);

  info.GetReturnValue().Set(Nan::New<Number>(-err));
}


NAN_METHOD(Ioctx::rmxattr) {
  if (info.Length() < 2 ||
      !info[0]->IsString() ||
      !info[1]->IsString()) {
    return Nan::ThrowError("Bad argument.");
  }

  Ioctx* obj = Nan::ObjectWrap::Unwrap<Ioctx>(info.This());
  if ( !obj->require_created() ) info.GetReturnValue().Set(Nan::Null());
  String::Utf8Value oid(info[0]);
  String::Utf8Value name(info[1]);

  int err = rados_rmxattr(obj->ioctx, *oid, *name);

  info.GetReturnValue().Set(Nan::New<Number>(-err));
}


NAN_METHOD(Ioctx::getxattrs) {
  if (info.Length() < 1 ||
      !info[0]->IsString()) {
    return Nan::ThrowError("Bad argument.");
  }

  Ioctx* obj = Nan::ObjectWrap::Unwrap<Ioctx>(info.This());
  if ( !obj->require_created() ) info.GetReturnValue().Set(Nan::Null());
  String::Utf8Value oid(info[0]);
  rados_xattrs_iter_t iter;

  Local<Object> xattrs = Nan::New<Object>();
  int err = rados_getxattrs(obj->ioctx, *oid, &iter);
  if (err < 0) {
    info.GetReturnValue().Set(Nan::Null());
  }
  while (1) {
      const char *name;
      const char *val;
      size_t len;

      err = rados_getxattrs_next(iter, &name, &val, &len);
      if (err < 0) {
        info.GetReturnValue().Set(Nan::Null());
      }
      if (name == NULL) {
          break;
      }

      xattrs->Set(Nan::New<String>(name).ToLocalChecked(), Nan::New<String>(val, len).ToLocalChecked());
  }
  rados_getxattrs_end(iter);

  info.GetReturnValue().Set(xattrs);
}


NAN_METHOD(Ioctx::stat) {
  if (info.Length() != 1 ||
      !info[0]->IsString()) {
    info.GetReturnValue().Set(Nan::Null());
  }

  Ioctx* obj = Nan::ObjectWrap::Unwrap<Ioctx>(info.This());
  if ( !obj->require_created() ) info.GetReturnValue().Set(Nan::Null());
  String::Utf8Value oid(info[0]);
  uint64_t psize;
  time_t pmtime;

  int err = rados_stat(obj->ioctx, *oid, &psize, &pmtime);
  if (err < 0) {
    info.GetReturnValue().Set(Nan::Null());
  }

  Local<Object> stat = Nan::New<Object>();
  stat->Set( Nan::New<String>("oid").ToLocalChecked(), Nan::New<String>(*oid).ToLocalChecked() );
  stat->Set( Nan::New<String>("psize").ToLocalChecked(), Nan::New<Number>(psize) );
  stat->Set( Nan::New<String>("pmtime").ToLocalChecked(), Nan::New<Number>(pmtime) );

  info.GetReturnValue().Set(stat);
}


void Ioctx::wait_complete(uv_work_t *req) {
  AsyncData* asyncdata = (AsyncData *)req->data;

  rados_aio_wait_for_complete(*asyncdata->comp);
  int ret = rados_aio_get_return_value(*asyncdata->comp);
  if (ret < 0) {
    asyncdata->err = -ret;
    asyncdata->size = 0;
  } else {
    asyncdata->size = ret;
  }

  rados_aio_release(*asyncdata->comp);
  delete asyncdata->comp;
}


void Ioctx::callback_complete(uv_work_t *req) {
  AsyncData *asyncdata = (AsyncData *)req->data;

  if (asyncdata->cb_buffer) {
    const unsigned argc = 2;
    Local<Value> argv[argc] = {
      Nan::Null(),
      Nan::NewBuffer(asyncdata->buffer, asyncdata->size).ToLocalChecked() };
    if (asyncdata->err) argv[0] = Nan::New<Number>(asyncdata->err);
    asyncdata->callback.Call(argc, argv);
  }
  else {
    const unsigned argc = 1;
    Local<Value> argv[argc] = {
      Nan::Null() };
    if (asyncdata->err) argv[0] = Nan::New<Number>(asyncdata->err);
    asyncdata->callback.Call(argc, argv);
  }

  delete asyncdata;
}


NAN_METHOD(Ioctx::aio_read) {
  if (info.Length() < 4 ||
      !info[0]->IsString() ||
      !info[3]->IsFunction()) {
    return Nan::ThrowError("Bad argument.");
  }

  Ioctx* obj = Nan::ObjectWrap::Unwrap<Ioctx>(info.This());
  if ( !obj->require_created() ) info.GetReturnValue().Set(Nan::Null());
  String::Utf8Value oid(info[0]);
  size_t size = info[1]->IsNumber() ? info[1]->IntegerValue() : 8192;
  uint64_t offset = info[2]->IsNumber() ? info[2]->IntegerValue() : 0;

  AsyncData *asyncdata = new AsyncData;
  char *buffer = new char[size];
  rados_completion_t *comp = new rados_completion_t;

  asyncdata->callback.SetFunction(info[3].As<Function>());
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

  return;
}


NAN_METHOD(Ioctx::aio_write) {
  if (info.Length() < 5 ||
      !info[0]->IsString() ||
      !Buffer::HasInstance(info[1]) ||
      !info[4]->IsFunction()) {
    return Nan::ThrowError("Bad argument.");
  }

  Ioctx* obj = Nan::ObjectWrap::Unwrap<Ioctx>(info.This());
  if ( !obj->require_created() ) info.GetReturnValue().Set(Nan::Null());
  String::Utf8Value oid(info[0]);
  char* buffer = Buffer::Data(info[1]);
  size_t size = info[2]->IsNumber() ? info[2]->Uint32Value() : Buffer::Length(info[1]);
  uint64_t offset = info[3]->IsNumber() ? info[3]->IntegerValue() : 0;

  AsyncData *asyncdata = new AsyncData;
  rados_completion_t *comp = new rados_completion_t;

  asyncdata->callback.SetFunction(info[4].As<Function>());
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

  return;
}


NAN_METHOD(Ioctx::aio_append) {
  if (info.Length() < 4 ||
      !info[0]->IsString() ||
      !Buffer::HasInstance(info[1]) ||
      !info[3]->IsFunction()) {
    return Nan::ThrowError("Bad argument.");
  }

  Ioctx* obj = Nan::ObjectWrap::Unwrap<Ioctx>(info.This());
  if ( !obj->require_created() ) info.GetReturnValue().Set(Nan::Null());
  String::Utf8Value oid(info[0]);
  char* buffer = Buffer::Data(info[1]);
  size_t size = info[2]->IsNumber() ? info[2]->Uint32Value() : Buffer::Length(info[1]);

  AsyncData *asyncdata = new AsyncData;
  rados_completion_t *comp = new rados_completion_t;

  asyncdata->callback.SetFunction(info[3].As<Function>());
  asyncdata->buffer = buffer;
  asyncdata->cb_buffer = false;
  asyncdata->size = size;
  asyncdata->comp = comp;
  asyncdata->err = 0;

  rados_aio_create_completion(NULL, NULL, NULL, comp);
  int err = rados_aio_append(obj->ioctx, *oid, *comp, buffer, size);
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

  return;
}


NAN_METHOD(Ioctx::aio_write_full) {
  if (info.Length() < 4 ||
      !info[0]->IsString() ||
      !Buffer::HasInstance(info[1]) ||
      !info[3]->IsFunction()) {
    return Nan::ThrowError("Bad argument.");
  }

  Ioctx* obj = Nan::ObjectWrap::Unwrap<Ioctx>(info.This());
  if ( !obj->require_created() ) info.GetReturnValue().Set(Nan::Null());
  String::Utf8Value oid(info[0]);
  char* buffer = Buffer::Data(info[1]);
  size_t size = info[2]->IsNumber() ? info[2]->Uint32Value() : Buffer::Length(info[1]);

  AsyncData *asyncdata = new AsyncData;
  rados_completion_t *comp = new rados_completion_t;

  asyncdata->callback.SetFunction(info[3].As<Function>());
  asyncdata->buffer = buffer;
  asyncdata->cb_buffer = false;
  asyncdata->size = size;
  asyncdata->comp = comp;
  asyncdata->err = 0;

  rados_aio_create_completion(NULL, NULL, NULL, comp);
  int err = rados_aio_write_full(obj->ioctx, *oid, *comp, buffer, size);
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

  return;
}

NAN_METHOD(Ioctx::aio_flush) {
  Ioctx* obj = Nan::ObjectWrap::Unwrap<Ioctx>(info.This());
  if ( !obj->require_created() ) info.GetReturnValue().Set(Nan::Null());

  int err = rados_aio_flush(obj->ioctx);

  info.GetReturnValue().Set(Nan::New<Number>(-err));
}


NAN_METHOD(Ioctx::aio_flush_async) {
  if (info.Length() < 1 ||
      !info[0]->IsFunction()) {
    return Nan::ThrowError("Bad argument.");
  }

  Ioctx* obj = Nan::ObjectWrap::Unwrap<Ioctx>(info.This());
  if ( !obj->require_created() ) info.GetReturnValue().Set(Nan::Null());

  AsyncData *asyncdata = new AsyncData;
  rados_completion_t *comp = new rados_completion_t;

  asyncdata->callback.SetFunction(info[0].As<Function>());
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

  return;
}

#define ENOENT 2
NAN_METHOD(Ioctx::objects_list) {
  Ioctx* obj = Nan::ObjectWrap::Unwrap<Ioctx>(info.This());
  if ( !obj->require_created() ) info.GetReturnValue().Set(Nan::Null());

  rados_list_ctx_t h_ctx;
  //Start listing objects in a pool.
  int err = rados_objects_list_open(obj->ioctx, &h_ctx);
  if (err < 0) {
    return Nan::ThrowError("open list failed.");
  }

  Local<Array> ret_list = Nan::New<Array>();
  uint32_t array_id = 0;
  //Get the next object name and locator in the pool.

  while(0 <= err) {
    const char *obj_name;
    err = rados_objects_list_next(h_ctx, &obj_name, NULL);
    if (err == 0) {
      ret_list->Set(array_id, Nan::New(obj_name).ToLocalChecked());
      array_id++;
    }
  }
  rados_objects_list_close(h_ctx);

  if (err < 0 && err != -ENOENT) {
    return Nan::ThrowError("list_next failed.");
  }

  info.GetReturnValue().Set(ret_list);
}
