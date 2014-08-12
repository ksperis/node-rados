#ifndef RADOS_H
#define RADOS_H

#include <node.h>
#include <node_buffer.h>
#include </usr/include/rados/librados.h>


class Rados : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> target);
  rados_t cluster;

 private:
  Rados();
  ~Rados();

  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> connect(const v8::Arguments& args);
  static v8::Handle<v8::Value> shutdown(const v8::Arguments& args);
  static v8::Handle<v8::Value> get_fsid(const v8::Arguments& args);
  static v8::Handle<v8::Value> pool_list(const v8::Arguments& args);
  static v8::Persistent<v8::Function> constructor;
};

class Ioctx : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> target);
  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  rados_ioctx_t ioctx;

 private:
  Ioctx();
  ~Ioctx();
  
  typedef struct AsyncData {
    v8::Persistent<v8::Function> callback;
    char* buffer;
    int err;
  } AsyncData;

  static void ack_callback(rados_completion_t comp, void *arg);

  static v8::Handle<v8::Value> destroy(const v8::Arguments& args);
  static v8::Handle<v8::Value> read(const v8::Arguments& args);
  static v8::Handle<v8::Value> write_full(const v8::Arguments& args);
  static v8::Handle<v8::Value> remove(const v8::Arguments& args);
  static v8::Handle<v8::Value> stat(const v8::Arguments& args);
  static v8::Handle<v8::Value> aio_write(const v8::Arguments& args);
  static v8::Handle<v8::Value> aio_append(const v8::Arguments& args);
  static v8::Handle<v8::Value> aio_write_full(const v8::Arguments& args);
  static v8::Handle<v8::Value> aio_read(const v8::Arguments& args);
  static v8::Handle<v8::Value> aio_read2(const v8::Arguments& args);
  static v8::Persistent<v8::Function> constructor;
};


#endif
