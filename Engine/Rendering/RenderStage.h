#ifndef __RENDER_STAGE_H__
#define __RENDER_STAGE_H__

#include "RenderContext.h"
#include "RenderingCamera.h"
#include "Tasks\WorkQueue.h"
#include "Container\Vector.h"
#include "Tasks\OsEvent.h"
#include "RenderView.h"
#include "Spatial.h"

/*
	Renderpath stage
*/
class RenderStage
{
private:
	// initial stages
	int Initial();
protected:
	RenderContext* Context;
	RenderInterface* Interface;

public:
	RenderStage(RenderContext* Context);
	virtual ~RenderStage();
	// execute stage
	virtual int Execute(RenderingCamera* Camera, Spatial* spatial, RenderQueue* renderQueue, WorkQueue* Queue, Vector<OsEvent*>& Events);
	// end
	virtual int End() { return 0; };
};

#endif
