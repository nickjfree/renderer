#ifndef __OBJECT__
#define __OBJECT__

#include "Shortcuts.h"
#include "Str.h"
#include "Context.h"
#include "Allocater.h"

// typedefine 

class Context;
// Base class of GameObject, Component, SubSystem, eg. with TypeHash, TypeName, and Id
class Object
{
	BASEOBJECT(Object);
	OBJECT(Object);
protected:
	Context * context;
	unsigned int RefCount;
	unsigned int ObjectId;
public:
	Object(Context * context_);
	virtual ~Object();
	int AddRef();
	int DecRef();
	virtual unsigned int GetObjectId();
	virtual void SetObjectId(unsigned int);
};

#endif