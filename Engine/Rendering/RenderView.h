#ifndef __RENDERVIEW__
#define __RENDERVIEW__

#include "RenderingCamera.h"
#include "RenderDesc.h"
#include "Core\Allocater.h"
#include "Core\Shortcuts.h"
#include "Container\RecyclePool.h"
#include "Container\Vector.h"
#include "Container\Dict.h"
#include "RenderObject.h"
#include "BatchCompiler.h"
#include "RenderQueue.h"
#include "Tasks\OsEvent.h"

/*
	Linkage of a camera and renderstage and rendertarget
*/

#define COMMANDBUFFER_SIZE 4194304 

class RenderView
{
	DECLARE_ALLOCATER(RenderView);
	DECLARE_RECYCLE(RenderView);
public:
	// camera
	RenderingCamera* Camera;
	// targets count
	int TargetCount;
	// targets
	int Targets[RENDER_MAX_TARGET];
	// clear depth
	int ClearDepth;
	// clear terget
	int ClearTargets;
	// depth
	int Depth;
	// type
	int Type;
	// view index
	int Index;
	// object list
	Vector<Node*> VisibleObjects;
	// command buffer, a binary rendercommand buffer. 
	// excutes by renderer, alloced with new is enough because it is recycled
	void* CommandBuffer;
	// Compiler
	BatchCompiler* Compiler;
	// RenderQueue
	RenderQueue* Queue;
	// Event
	OsEvent* Event;
	// parameters
	Dict Parameters;
public:
	RenderView();
	virtual ~RenderView();
	virtual int Compile(RenderContext* Context);
	int QueueCommand();
};


#endif
