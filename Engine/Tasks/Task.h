#ifndef __TASK__
#define __TASK__

/*
	Asyns Task Item
*/

#include "Container\LinkList.h"
#include "Core\Allocater.h"
#include "Container\RecyclePool.h"
#include  "Core\Shortcuts.h"

class Task
{
	DECLARE_RECYCLE(Task);
	DECLARE_ALLOCATER(Task);
public:
	LinkList<Task> TaskList;
public:
	Task();
	virtual ~Task();
	// work, called in worker thread
	virtual int Work();
	// complete called in main thread
	virtual int Complete();
	// failed called in main thread
	virtual int Failed();
};

#endif