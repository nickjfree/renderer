#ifndef __OBJECT_FACTORY__
#define __OBJECT_FACTORY__


#include <memory>
#include "Object.h"

using std::shared_ptr;

class ObjectFactory
{
public:
	ObjectFactory()	{}
	~ObjectFactory() {}
	virtual void * CreateObject(Context * context) = 0;
	virtual void DestoryObject(void * Object) = 0;
};


template <class T> 
class ObjectFactoryImp : public ObjectFactory
{
public:
	ObjectFactoryImp() {}
	~ObjectFactoryImp() {}
	virtual void * CreateObject(Context * context) {
		return new T(context);
	}

	virtual void DestoryObject(void * Object) {
		T * obj = (T*)Object;
		delete obj;
	}
};




#endif
