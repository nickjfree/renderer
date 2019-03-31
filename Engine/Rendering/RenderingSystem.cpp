#include "RenderingSystem.h"
#include "RenderView.h"
#include "CullingTask.h"
#include "Tasks\WorkQueue.h"
#include "Tasks\OsEvent.h"
#include "Resource\ResourceCache.h"
#include "RenderPreloading.h"
#include "Rendering\MeshRenderer.h"
#include "Rendering\Light.h"


RenderingSystem::RenderingSystem(Context * context) : System(context), frames(0), MainCamera(0)
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
	BasicCullingImp * Basic = new BasicCullingImp();
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
	WorkQueue * Queue = context->GetSubsystem<WorkQueue>();
	Control->Queue = Queue;
	Control->spatial = spatial;
}

void RenderingSystem::PreloadingResource() {
	ResourceCache * Cache = context->GetSubsystem<ResourceCache>();
	// loading ssao
	Preloadings[0] = "Material\\Materials\\ssao.xml\\0";
	Variant Param;
	Param.as<String*>() = &Preloadings[0];
	RenderPreloading * Preloading = new RenderPreloading(context);
	Cache->AsyncLoadResource(Preloadings[0], Preloading, Param);
	// loading hdr shaders
	Preloadings[1] = "Shader\\shaders\\HDR\\0";
	Param.as<String*>() = &Preloadings[1];
	Preloading = new RenderPreloading(context);
	Cache->AsyncLoadResource(Preloadings[1], Preloading, Param);
    // loading oit shaders
    Preloadings[2] = "Shader\\shaders\\OIT\\0";
    Param.as<String*>() = &Preloadings[2];
    Preloading = new RenderPreloading(context);
    Cache->AsyncLoadResource(Preloadings[2], Preloading, Param);

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
	context->RegisterObject<Renderer>();
	context->RegisterObject<Light>();
	return 0;
}

int RenderingSystem::Update(int ms) {
	if (!MainCamera) {
		return -1;
	}
	// MainCamera->Update(ms);
	// rendercontrol takes over the rendering process
	//disable rendering
	Control->Execute();
	return 0;
}

int RenderingSystem::Shutdown() {
	// do nothing
	return 0;
}

void RenderingSystem::SetSpatial(Spatial * spatial_) {
	if (spatial)
	{
		delete spatial;
	}
	spatial = spatial_;
	Control->spatial = spatial;
}