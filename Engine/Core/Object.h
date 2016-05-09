#ifndef __OBJECT__
#define __OBJECT__

#include "Shortcuts.h"
#include "Str.h"
#include "Context.h"
#include "Allocater.h"


// Base class of GameObject, Component, SubSystem, eg. with TypeHash, TypeName, and Id
class Object
{
	BASEOBJECT(Object);
	OBJECT(Object);
private:
	unsigned int RefCount;
protected:
	Context * context;
public:
	Object(Context * context_);
	virtual ~Object();
	int AddRef();
	int DecRef();
};

#endif