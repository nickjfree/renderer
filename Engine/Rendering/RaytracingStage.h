#ifndef __RAYTRACING_STAGE__
#define __RAYTRACING_STAGE__


#include "RenderStage.h"
#include "ShaderLibrary.h"
#include "RenderView.h"

struct Viewport
{
	float left;
	float top;
	float right;
	float bottom;
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
	// shaderlibraries
	ShaderLibrary* TestShader;
	// rtTarget
	int rtTarget;
	// Parameters for rt stage
	Dict Parameter;
	// renderviews
	Vector<RenderView*> RenderViews;
	// test
	RayGenConstantBuffer rayGenCB;
public:
	RaytracingStage(RenderContext* Context);
	virtual ~RaytracingStage();

	// execute stage
	virtual int Execute(RenderingCamera* Camera, Spatial* spatial, RenderQueue* renderQueue, WorkQueue* Queue, Vector<OsEvent*>& Events);
	// end
	virtual int End();
};


#endif
