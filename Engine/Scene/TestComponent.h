#ifndef __TEST_COMPONONT__
#define __TEST_COMPONONT__

#include "Component.h"

class TestComponent : public Component
{

	//private: 
	//	static Allocater<TestComponent> m_Allocater;
	//public: 
	//		void * operator new(size_t size){ return m_Allocater.Alloc(); } 
	//		void operator delete(void * ptr) { m_Allocater.Free(ptr); }
private:
	int a[100];
public:
	OBJECT(TestComponent);
	BASEOBJECT(Component);
	DECLARE_ALLOCATER(TestComponent);
	TestComponent(Context* context_);
	~TestComponent();
	virtual int HandleEvent(Event* Evt);
};

#endif


