//#define BUILDING_NODE_EXTENSION
#include <node.h>
#include "rados.h"

using namespace v8;

void InitAll(Handle<Object> exports) {
  Rados::Init(exports);
  Ioctx::Init(exports);
}

NODE_MODULE(rados, InitAll)
