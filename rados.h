#ifndef RADOS_H
#define RADOS_H

#include <node.h>
#include <node_buffer.h>
#include </usr/include/rados/librados.h>

#include <nan.h>

const size_t DEFAULT_BUFFER_SIZE = 1024;
const uint STATE_CREATED     = 1;
const uint STATE_CONFIGURED  = 2;
const uint STATE_CONNECTED   = 3;
const uint STATE_DESTROYED   = 4;

class Rados : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> target);
  rados_t cluster;

  bool require_connected();

 private:
  Rados();
  ~Rados(); 

  uint state;

  static NAN_METHOD(New);
  static NAN_METHOD(connect);
  static NAN_METHOD(shutdown);
  static NAN_METHOD(get_fsid);
  static NAN_METHOD(pool_create);
  static NAN_METHOD(pool_delete);
  static NAN_METHOD(pool_list);

  static v8::Persistent<v8::FunctionTemplate> constructor;
};

class Ioctx : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> target);
  static NAN_METHOD(New);
  rados_ioctx_t ioctx;

  bool require_created();

 private:
  Ioctx();
  ~Ioctx();
  
  typedef struct AsyncData {
    NanCallback callback;
    char* buffer;
    int cb_buffer;
    size_t size;
    uint64_t offset;
    int err;
    rados_completion_t* comp;
  } AsyncData;
 
  Rados* rados;
  uint state;
  
  static void callback_complete(uv_work_t *req);
  static void wait_complete(uv_work_t *req);

  static NAN_METHOD(pool_set_auid);
  static NAN_METHOD(pool_get_auid);
  static NAN_METHOD(destroy);
  static NAN_METHOD(snap_create);
  static NAN_METHOD(snap_remove);
  static NAN_METHOD(snap_rollback);
  static NAN_METHOD(read);
  static NAN_METHOD(write);
  static NAN_METHOD(write_full);
  static NAN_METHOD(clone_range);
  static NAN_METHOD(append);
  static NAN_METHOD(remove);
  static NAN_METHOD(trunc);
  static NAN_METHOD(stat);
  static NAN_METHOD(getxattr);
  static NAN_METHOD(setxattr);
  static NAN_METHOD(rmxattr);
  static NAN_METHOD(getxattrs);
  static NAN_METHOD(aio_read);
  static NAN_METHOD(aio_write);
  static NAN_METHOD(aio_append);
  static NAN_METHOD(aio_write_full);
  static NAN_METHOD(aio_flush);
  static NAN_METHOD(aio_flush_async);
  static NAN_METHOD(aio_objects_list);
  static v8::Persistent<v8::FunctionTemplate> constructor;
};


#endif
