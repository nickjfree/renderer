#ifndef __RESOURCETASK__
#define __RESOURCETASK__

#include "Tasks\Task.h"
#include "Resource\Resource.h"
#include "Resource\FileMappingLoader.h"
/*
	Resource Loading task
*/

class ResourceTask : public Task
{
	DECLAR_ALLOCATER(ResourceTask);
	DECLAR_RECYCLE(ResourceTask);
public:
	Resource * resource;
	Resource * caller;
	Variant Param;
public:
	ResourceTask();
	virtual ~ResourceTask();

	virtual int Work();

	virtual int Complete();
};

#endif