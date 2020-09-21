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
	// raytracing rendertarget
	int rtTarget;
	// accumulate buffers(color and moments)
	int AccColor[2];
	int AccMoments[2];
	// Parameters for rt stage
	Dict Parameter;
	// renderviews
	Vector<RenderView*> RenderViews;
	// test
	RayGenConstantBuffer rayGenCB;
	// int frame count
	int NumFrames = 0;
	// denosing shader
	Shader* DenosingShader = nullptr;
private:
	// build scene
	int BuildRaytracingScene(RenderingCamera* Camera, Spatial* spatial, BatchCompiler* compiler);
	// temporal_accumulation
	int Accumulation(BatchCompiler* Compiler);
	// filter
	int Filter(BatchCompiler* Compiler);
	// raytracing
	int Raytracing(RenderingCamera* Camera, Spatial* spatial, BatchCompiler* compiler);
	// denosing
	int Denosing(BatchCompiler* Compiler);
public:
	RaytracingStage(RenderContext* Context);
	virtual ~RaytracingStage();

	// execute stage
	virtual int Execute(RenderingCamera* Camera, Spatial* spatial, RenderQueue* renderQueue, WorkQueue* Queue, Vector<OsEvent*>& Events);
	// end
	virtual int End();
};


#endif
