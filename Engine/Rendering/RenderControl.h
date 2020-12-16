#ifndef __RENDER_CTL__
#define __RENDER_CTL__

#include "RenderingPath.h"
#include "Spatial.h"
#include "BasicPartition.h"
#include "RenderQueue.h"
#include "RenderProcesser.h"
#include "Tasks\OsEvent.h"
#include "RenderingCamera.h"
#include "Tasks\WorkQueue.h"
#include "FrameGraph.h"

/*
	render control
*/

#define MAX_RENDER_PATH 8


class RenderControl
{
private:
	RenderingPath* RenderPath[MAX_RENDER_PATH];
	RenderContext* Context;
	RenderInterface* Interface;
	Vector<RenderingCamera*> Cameras;
	RenderQueue* RenderQueue_;
	// render processer
	RenderProcesser* RenderProcesser_;
	// OsEvent for frame sync
	int EventCount = 0;
	Vector<OsEvent*> Events;
	// test rendertarget
	int DepthBuffer;
	// test Coler buffer
	int ColorBuffer;
private:
	void StartCamera(RenderingCamera* Camera);
public:
	Spatial* spatial;
	// workqueue
	WorkQueue* Queue;
	enum  RenderPath {
		FORWARD,
		DEFERRED,
		LIGHT_PRE,
	};
public:
	RenderControl(RenderContext* Context);
	virtual ~RenderControl();
	int Initialize();
	int Execute();
	int AddCamera(RenderingCamera* Camera);
	int RemoveCamera(RenderingCamera* Camera);
	void SetSpatial(Spatial* spatial_) { spatial = spatial_; }
};

#endif
