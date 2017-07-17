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
	unsigned int ret = InterlockedDecrement(&RefCount);
	if (ret == 0) {
		// time to free the object
		delete this;
	}
	return RefCount;
}

unsigned int Object::GetObjectId() {
	return ObjectId;
}

void Object::SetObjectId(unsigned int id) {
	ObjectId = id;
}

