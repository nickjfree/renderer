#ifndef __CULLING_TASK__
#define __CULLING_TASK__


#include "Tasks\Task.h"
#include "RenderView.h"
#include "Spatial.h"
#include "Tasks\OsEvent.h"
#include "Core\Shortcuts.h"

/*
	Task for querying visible objects from scene and then generating command buffer from them
*/

class CullingTask : public Task
{
	DECLARE_ALLOCATER(CullingTask);
	DECLARE_RECYCLE(CullingTask);
public:
	// renderview 
	RenderView* renderview;
	// spatial, from which to cull
	Spatial* spatial;
	// objects type
	int ObjectType;
	// event
	OsEvent* Event;
	// render context
	RenderContext* Context;
public:
	CullingTask();
	~CullingTask();

	virtual int Work();

	virtual int Complete();
};

#endif