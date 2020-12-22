#include "RenderingSystem.h"
#include "RenderView.h"
#include "CullingTask.h"
#include "Tasks\WorkQueue.h"
#include "Tasks\OsEvent.h"
#include "Resource\ResourceCache.h"
#include "RenderPreloading.h"
#include "Rendering\MeshRenderer.h"
#include "Rendering\Light.h"
#include "Terrain.h"


RenderingSystem::RenderingSystem(Context* context) : System(context), frames(0)
{
}


RenderingSystem::~RenderingSystem()
{
}

void RenderingSystem::InitRendering() {
	RenderContext_->RegisterRenderTarget("Color", 0);
}

void RenderingSystem::InitLowLevel() {
	Interface->Initialize(RenderContext_->FrameWidth, RenderContext_->FrameHeight);
	BasicCullingImp* Basic = new BasicCullingImp();
	spatial = Basic;
}

void RenderingSystem::InitTopLevel() {
	// effect and some default stuff
	// render stat register
	// Init all the default stuff heress
	InitRendering();
	// init control
	Control = new RenderControl(RenderContext_);
	Control->Initialize();
	WorkQueue* Queue = context->GetSubsystem<WorkQueue>();
	Control->Queue = Queue;
	Control->spatial = spatial;
}

void RenderingSystem::PreloadingResource() {
	ResourceCache* Cache = context->GetSubsystem<ResourceCache>();
	// loading ssao
	Preloadings[0] = "Material\\Materials\\ssao.xml\\0";
	Variant Param;
	Param.as<String*>() = &Preloadings[0];
	RenderPreloading* Preloading = new RenderPreloading(context);
	Cache->AsyncLoadResource(Preloadings[0], Preloading, Param);
	// loading hdr shaders
	Preloadings[1] = "Shader\\shaders\\HDR\\0";
	Param.as<String*>() = &Preloadings[1];
	Cache->AsyncLoadResource(Preloadings[1], Preloading, Param);
	// loading oit shaders
	Preloadings[2] = "Shader\\shaders\\OIT\\0";
	Param.as<String*>() = &Preloadings[2];
	Cache->AsyncLoadResource(Preloadings[2], Preloading, Param);
	// load reflection materials
	Preloadings[3] = "Material\\Materials\\reflection.xml\\0";
	Param.as<String*>() = &Preloadings[3];
	Cache->AsyncLoadResource(Preloadings[3], Preloading, Param);
	// load  denoising shaders
	Preloadings[4] = "Shader\\shaders\\Denoising\\0";
	Param.as<String*>() = &Preloadings[4];
	Cache->AsyncLoadResource(Preloadings[4], Preloading, Param);
}

int RenderingSystem::Initialize() {
	// init interface
	Interface = new RenderImp();
	// init context
	RenderContext_ = new RenderContext(Interface);
	// init render queue
	RenderQueue_ = new RenderQueue();
	// init render processer
	RenderProcesser_ = new RenderProcesser(RenderContext_);
	// init renderengine
	InitLowLevel();
	InitTopLevel();
	// register component
	// context->RegisterObject<Renderer>();
	context->RegisterObject<Light>();
	context->RegisterObject<MeshRenderer>();
	context->RegisterObject<Terrain>();
	context->RegisterObject<TerrainNode>();

	// for test
	// auto render = new D3D12RenderInterface();
	// render->Initialize(3840, 2160);
	return 0;
}

int RenderingSystem::Update(int ms) {
	// profile
	PIXScopedEvent(0xFF00FF00, __FUNCTION__);
	// rendercontrol takes over the rendering process
	//disable rendering
	Control->Execute();
	return 0;
}

int RenderingSystem::Shutdown() {
	// do nothing
	return 0;
}

void RenderingSystem::SetSpatial(Spatial* spatial_) {
	spatial = spatial_;
	Control->spatial = spatial;
}