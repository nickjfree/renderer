#ifndef __TEST_COMPONONT__
#define __TEST_COMPONONT__

#include "ComponontTemplate.h"
class TestComponont : public ComponontTemplate<TestComponont>
{
private:
	int a[100];
public:
	TestComponont();
	~TestComponont();
};

#endif


