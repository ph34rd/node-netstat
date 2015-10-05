#include <node.h>
#include <nan.h>
#include "get.h"

using v8::FunctionTemplate;
using v8::Handle;
using v8::Object;
using v8::String;

void init(Handle<Object> exports, Handle<Object> module) {
	module->Set(
		NanNew<String>("exports"),
		NanNew<FunctionTemplate>(get)->GetFunction()
	);
}

NODE_MODULE(netstat, init)
