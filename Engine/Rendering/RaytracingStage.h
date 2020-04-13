#ifndef __RAYTRACING_STAGE__
#define __RAYTRACING_STAGE__


#include "RenderStage.h"
#include "ShaderLibrary.h"

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
public:
	RaytracingStage(RenderContext* Context);
	virtual ~RaytracingStage();

	// execute stage
	virtual int Execute(RenderingCamera* Camera, Spatial* spatial, RenderQueue* renderQueue, WorkQueue* Queue, Vector<OsEvent*>& Events);
	// end
	virtual int End();
};


#endif
