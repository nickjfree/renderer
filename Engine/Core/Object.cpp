#include "Object.h"
#include <Windows.h>

Object::Object(Context * context_) :RefCount(0), context(context_)
{
}


Object::~Object()
{
}

int Object::AddRef() {
	InterlockedIncrement(&RefCount);
	return RefCount;
}

int Object::DecRef() {
	InterlockedDecrement(&RefCount);
	return RefCount;
}

