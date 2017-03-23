#include "Object.h"
#include <Windows.h>


Object::Object(Context * context_) :RefCount(0), context(context_), ObjectId(-1) {
}


Object::~Object() {
}

int Object::AddRef() {
	InterlockedIncrement(&RefCount);
	return RefCount;
}

int Object::DecRef() {
	InterlockedDecrement(&RefCount);
	return RefCount;
}

unsigned int Object::GetObjectId() {
	return ObjectId;
}

void Object::SetObjectId(unsigned int id) {
	ObjectId = id;
}

