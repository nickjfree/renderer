#ifndef __RAYTRACING_STAGE__
#define __RAYTRACING_STAGE__


#include "RenderStage.h"
#include "ShaderLibrary.h"
#include "RenderView.h"

struct Viewport
{
	float left = -100;
	float top = -100;
	float right = 100; 
	float bottom = 100;
};

struct RayGenConstantBuffer
{
	Viewport viewport;
	Viewport stencil;
};


class RaytracingStage : public RenderStage
{
private:
	// ratrcing objects (renderObjects)
	Vector<Node*> rtInstances;
private:
	// initialization
	void Initialize();
	// rtTarget
	int rtTarget;
	// Parameters for rt stage
	Dict Parameter;
	// renderviews
	Vector<RenderView*> RenderViews;
	// test
	RayGenConstantBuffer rayGenCB;
	// int frame count
	int NumFrames = 0;
public:
	RaytracingStage(RenderContext* Context);
	virtual ~RaytracingStage();

	// execute stage
	virtual int Execute(RenderingCamera* Camera, Spatial* spatial, RenderQueue* renderQueue, WorkQueue* Queue, Vector<OsEvent*>& Events);
	// end
	virtual int End();
};


#endif
