#ifndef __TEST_COMPONONT__
#define __TEST_COMPONONT__

#include "ComponentTemplate.h"
class TestComponent : public ComponentTemplate<TestComponent>
{
private:
	int a[100];
public:
	TestComponent();
	~TestComponent();
};

#endif


