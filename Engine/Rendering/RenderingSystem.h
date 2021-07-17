#ifndef __RENDERING_SYS__
#define __RENDERING_SYS__

#include "Core/System.h"
#include "RenderInterface.h"
//#include "RenderEngine.h"
#include "RenderContext.h"
#include "RenderControl.h"

#ifdef RENDER_D3D11
#include "D3D11Render\D3D11Render.h"

using D3D11API::D3D11Render;

typedef D3D11Render RenderImp;
#endif

#ifdef RENDER_D3D12
//#include "D3D12Render\D3D12Render.h"
//using D3D12API::D3D12Render;
////typedef D3D12Render RenderImp;
#endif


// test
#include "D3D12Renderer\D3D12Renderer.h"
using D3D12Renderer::D3D12RenderInterface;
using D3D12Renderer::D3D12CommandContext;
typedef D3D12RenderInterface RenderImp;

//#else
//typedef RenderInterface RenderImp;
//#endif

#include "RenderingCamera.h"
#include "Spatial.h"
#include "BasicCullingImp.h"
#include "QuadTreeImp.h"

// renderqueue imp
#include "RenderQueue.h"
#include "RenderProcesser.h"
#include "Tasks\OsEvent.h"

/*
	Rendering system. implement with H3D rendering engine
*/

class RenderingSystem : public System {
	BASEOBJECT(System);
	OBJECT(RenderingSystem);
private:
	// frame
	unsigned int frames;
	// Spatial
	Spatial* spatial;
	// renderinterface
	RenderInterface* Interface;
	// render context
	RenderContext* RenderContext_;
	// Rendercontrol
	RenderControl* Control = nullptr;
	// render queue
	RenderQueue* RenderQueue_ = nullptr;
	// render processer
	RenderProcesser* RenderProcesser_ = nullptr;
	// test rendertarget
	int DepthBuffer = -1;
	// test Coler buffer
	int ColorBuffer = -1;
	// Preloadings
	String Preloadings[16];
private:
	// set up rendering 
	void InitLowLevel();
	// set up lighting, ssao. post processing. basic materials and geometry
	void InitTopLevel();
	// init render ddfault rendering setup
	void InitRendering();
public:
	RenderingSystem(Context* context);
	virtual ~RenderingSystem();
	// init
	virtual int Initialize();
	// update
	virtual int Update(int ms);
	// shutdown 
	virtual int Shutdown();
	// get interface
	RenderInterface* GetRenderInterface() { return Interface; }
	// get render context
	RenderContext* GetRenderContext() { return RenderContext_; }
	// get spatial
	Spatial* GetSpatial() { return spatial; }
	// set spatial
	void SetSpatial(Spatial* spatial);
	// get renderqueue
	RenderQueue* GetRenderQueue() { return RenderQueue_; }
	// set main camera
	void AddCamera(RenderingCamera* Camera) { Control->AddCamera(Camera); }
	// remove camera
	void RemoveCamera(RenderingCamera* Camera) { Control->RemoveCamera(Camera); }
	// preloading
	// preloading resource
	void PreloadingResource();
};

#endif