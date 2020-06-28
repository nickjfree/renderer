#include "PrepassStage.h"
#include "CullingTask.h"

PrepassStage::PrepassStage(RenderContext* Context) :RenderStage(Context)
{
	Initial();
}


PrepassStage::~PrepassStage()
{
}

// create g-buffer, for opaque objects
void PrepassStage::CreateGBuffer() {
	R_TEXTURE2D_DESC desc = {};
	desc.Width = Context->FrameWidth;
	desc.Height = Context->FrameHeight;
	desc.ArraySize = 1;
	desc.CPUAccess = (R_CPU_ACCESS)0;
	desc.BindFlag = (R_BIND_FLAG)(BIND_RENDER_TARGET | BIND_SHADER_RESOURCE);
	desc.MipLevels = 1;
	desc.Usage = DEFAULT;
	desc.Format = FORMAT_R32_FLOAT;
	desc.SampleDesc.Count = 1;
	// linearz
	Targets[0] = Interface->CreateTexture2D(&desc, 0, 0, 0);
	// normal
	desc.Format = FORMAT_R16G16_FLOAT;
	Targets[1] = Interface->CreateTexture2D(&desc, 0, 0, 0);
	// motion vector
	desc.Format = FORMAT_R16G16B16A16_FLOAT;
	Targets[2] = Interface->CreateTexture2D(&desc, 0, 0, 0);
	// albedo
	desc.Format = FORMAT_R8G8B8A8_UNORM_SRGB;
	Targets[3] = Interface->CreateTexture2D(&desc, 0, 0, 0);
	// specular/reghness/metalic
	desc.Format = FORMAT_R8G8B8A8_UNORM;
	Targets[4] = Interface->CreateTexture2D(&desc, 0, 0, 0);

	// CompactData ,  objectid/depth/normal.x/normal.y
	desc.Format = FORMAT_R16G16B16A16_FLOAT;
	CompactData[0] = Targets[5] = Interface->CreateTexture2D(&desc, 0, 0, 0);
	CompactData[1] = Targets[6] = Interface->CreateTexture2D(&desc, 0, 0, 0);


	desc.BindFlag = BIND_DEPTH_STENCIL;
	desc.Format = FORMAT_D24_UNORM_S8_UINT;
	Depth = Interface->CreateTexture2D(&desc, NULL, 0, 0);
	// resgister targets
	Context->RegisterRenderTarget("gDepthBuffer", Targets[0]);
	Context->RegisterRenderTarget("gNormalBuffer", Targets[1]);

	Context->RegisterRenderTarget("gNormalBuffer1", Targets[1]);

	Context->RegisterRenderTarget("gMotionVector", Targets[2]);
	Context->RegisterRenderTarget("gDiffuseBuffer", Targets[3]);
	Context->RegisterRenderTarget("gSpecularBuffer", Targets[4]);
	Context->RegisterRenderTarget("Depth", Depth);
}

// create a-buffer, for transparency objects
void PrepassStage::CreateABuffer() {

	// clear mask texture
	R_TEXTURE2D_DESC desc = {};
	desc.Width = Context->FrameWidth;
	desc.Height = Context->FrameHeight;
	desc.ArraySize = 1;
	desc.CPUAccess = (R_CPU_ACCESS)0;
	desc.BindFlag = (R_BIND_FLAG)(BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS);
	desc.MipLevels = 1;
	desc.Usage = DEFAULT;
	desc.Format = FORMAT_R32_UINT;
	desc.SampleDesc.Count = 1;
	ABuffers[0] = Interface->CreateTexture2D(&desc, 0, 0, 0);

	// uav for depth and color
	DWORD Pixels = Context->FrameWidth * Context->FrameHeight;
	int nodes = 4;
	R_BUFFER_DESC buffer_desc = {};
	buffer_desc.BindFlags = (R_BIND_FLAG)(BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS);
	buffer_desc.CPUAccessFlags = (R_CPU_ACCESS)0;
	buffer_desc.MiscFlags = (R_MISC)0;
	buffer_desc.Size = Pixels * 4 * nodes;
	buffer_desc.StructureByteStride = 4 * nodes;
	buffer_desc.Usage = DEFAULT;
	buffer_desc.CPUData = nullptr;
	ABuffers[1] = Interface->CreateBuffer(&buffer_desc);
	ABuffers[2] = Interface->CreateBuffer(&buffer_desc);

	// resgister targets
	Variant Resource;
	Resource.as<int>() = ABuffers[0];
	Context->SetResource("gAOITSPClearMaskUAV", Resource);
	Context->SetResource("gAOITSPClearMaskSRV", Resource);
	Resource.as<int>() = ABuffers[1];
	Context->SetResource("gAOITSPDepthDataUAV", Resource);
	Context->SetResource("gAOITSPDepthDataSRV", Resource);
	Resource.as<int>() = ABuffers[2];
	Context->SetResource("gAOITSPColorDataUAV", Resource);
	Context->SetResource("gAOITSPColorDataSRV", Resource);
}

void PrepassStage::CreateRenderState() {
	R_DEPTH_STENCIL_DESC depth = {};
	// two side stencil for liging pass
	depth.ZTestEnable = 1;
	depth.ZWriteEnable = 0;
	depth.DepthFunc = R_CMP_FUNC::LESS;
	depth.StencilEnable = 1;
	depth.StencilFailFront = R_STENCIL_OP::KEEP;
	depth.DepthFailFront = R_STENCIL_OP::DECR;
	depth.StencilPassFront = R_STENCIL_OP::KEEP;
	depth.StencilFuncFront = R_CMP_FUNC::ALWAYS;
	depth.StencilFailBack = R_STENCIL_OP::KEEP;
	depth.DepthFailBack = R_STENCIL_OP::INCR;
	depth.StencilPassBack = R_STENCIL_OP::KEEP;
	depth.StencilFuncBack = R_CMP_FUNC::ALWAYS;
	depth.StencilRef = 1;
	DepthStat[0] = Interface->CreateDepthStencilStatus(&depth);
	// shading pass depth
	depth.ZTestEnable = 0;
	depth.ZWriteEnable = 0;
	depth.DepthFunc = R_CMP_FUNC::LESS;
	depth.StencilEnable = 1;
	depth.StencilFailFront = R_STENCIL_OP::KEEP;
	depth.DepthFailFront = R_STENCIL_OP::KEEP;
	depth.StencilPassFront = R_STENCIL_OP::ZERO;
	depth.StencilFuncFront = R_CMP_FUNC::NOT_EQUAL;
	depth.StencilFailBack = R_STENCIL_OP::KEEP;
	depth.DepthFailBack = R_STENCIL_OP::KEEP;
	depth.StencilPassBack = R_STENCIL_OP::ZERO;
	depth.StencilFuncBack = R_CMP_FUNC::NOT_EQUAL;
	depth.StencilRef = 0;
	DepthStat[1] = Interface->CreateDepthStencilStatus(&depth);
	// normal depth
	depth.ZTestEnable = 1;
	depth.ZWriteEnable = 1;
	depth.DepthFunc = R_CMP_FUNC::LESS_EQUAL;
	depth.StencilEnable = 0;
	depth.StencilFailFront = R_STENCIL_OP::KEEP;
	depth.DepthFailFront = R_STENCIL_OP::KEEP;
	depth.StencilPassFront = R_STENCIL_OP::ZERO;
	depth.StencilFuncFront = R_CMP_FUNC::NOT_EQUAL;
	depth.StencilFailBack = R_STENCIL_OP::KEEP;
	depth.DepthFailBack = R_STENCIL_OP::KEEP;
	depth.StencilPassBack = R_STENCIL_OP::ZERO;
	depth.StencilFuncBack = R_CMP_FUNC::NOT_EQUAL;
	depth.StencilRef = 0;
	DepthStat[2] = Interface->CreateDepthStencilStatus(&depth);
	// transparent pass depth stencil
	depth.ZTestEnable = 1;
	depth.ZWriteEnable = 0;
	depth.DepthFunc = R_CMP_FUNC::LESS_EQUAL;
	depth.StencilEnable = 1;
	depth.StencilFailFront = R_STENCIL_OP::KEEP;
	depth.DepthFailFront = R_STENCIL_OP::KEEP;
	depth.StencilPassFront = R_STENCIL_OP::REPLACE;
	depth.StencilFuncFront = R_CMP_FUNC::ALWAYS;
	depth.StencilFailBack = R_STENCIL_OP::KEEP;
	depth.DepthFailBack = R_STENCIL_OP::KEEP;
	depth.StencilPassBack = R_STENCIL_OP::REPLACE;
	depth.StencilFuncBack = R_CMP_FUNC::ALWAYS;
	depth.StencilRef = 1;
	DepthStat[3] = Interface->CreateDepthStencilStatus(&depth);
	// oit resolve/clear pass depth stencil
	depth.ZTestEnable = 0;
	depth.ZWriteEnable = 0;
	depth.DepthFunc = R_CMP_FUNC::LESS_EQUAL;
	depth.StencilEnable = 0;
	depth.StencilFailFront = R_STENCIL_OP::KEEP;
	depth.DepthFailFront = R_STENCIL_OP::KEEP;
	depth.StencilPassFront = R_STENCIL_OP::KEEP;
	depth.StencilFuncFront = R_CMP_FUNC::EQUAL;
	depth.StencilFailBack = R_STENCIL_OP::KEEP;
	depth.DepthFailBack = R_STENCIL_OP::KEEP;
	depth.StencilPassBack = R_STENCIL_OP::KEEP;
	depth.StencilFuncBack = R_CMP_FUNC::EQUAL;
	depth.StencilRef = 1;
	DepthStat[4] = Interface->CreateDepthStencilStatus(&depth);

	// culling pass blend state
	R_BLEND_STATUS blend = {};
	blend.Enable = 0;
	blend.SrcBlend = R_BLEND::BLEND_ONE;
	blend.DestBlend = R_BLEND::BLEND_ONE;
	blend.BlendOp = R_BLEND_OP::BLEND_OP_ADD;
	blend.SrcBlendAlpha = R_BLEND::BLEND_ONE;
	blend.DestBlendAlpha = R_BLEND::BLEND_ONE;
	blend.BlendOpAlpha = R_BLEND_OP::BLEND_OP_ADD;
	blend.Mask = R_BLEND_MASK::DISABLE_ALL;
	BlendStat[0] = Interface->CreateBlendStatus(&blend);
	// lighting pass addtive  blend
	blend.Enable = 1;
	blend.Mask = R_BLEND_MASK::ENABLE_ALL;
	BlendStat[1] = Interface->CreateBlendStatus(&blend);
	// normal stat
	blend.Enable = 0;
	blend.Mask = R_BLEND_MASK::ENABLE_ALL;
	BlendStat[2] = Interface->CreateBlendStatus(&blend);
	// alpha blend
	blend.Enable = 1;
	blend.SrcBlend = R_BLEND::BLEND_ONE;
	blend.DestBlend = R_BLEND::BLEND_SRC_ALPHA;
	blend.BlendOp = R_BLEND_OP::BLEND_OP_ADD;
	blend.SrcBlendAlpha = R_BLEND::BLEND_ZERO;
	blend.DestBlendAlpha = R_BLEND::BLEND_ZERO;
	blend.BlendOpAlpha = R_BLEND_OP::BLEND_OP_ADD;
	BlendStat[3] = Interface->CreateBlendStatus(&blend);
	// restar
	R_RASTERIZER_DESC raster = {};
	raster.CullMode = R_CULL::NONE;
	raster.FillMode = R_FILL::R_SOLID;
	raster.FrontCounterClockwise = 1;
	raster.AntialiasedLineEnable = 1;
	raster.MultisampleEnable = 1;
	RasterStat[0] = Interface->CreateRasterizerStatus(&raster);
	raster.CullMode = R_CULL::BACK;
	raster.FillMode = R_FILL::R_SOLID;
	raster.FrontCounterClockwise = 1;
	RasterStat[1] = Interface->CreateRasterizerStatus(&raster);
	// register
	Context->RegisterRenderState("TwoSideStencil", DepthStat[0]);
	Context->RegisterRenderState("NoZ", DepthStat[1]);
	Context->RegisterRenderState("Depth", DepthStat[2]);
	Context->RegisterRenderState("TransDepth", DepthStat[3]);
	Context->RegisterRenderState("ResolveDepth", DepthStat[4]);
	Context->RegisterRenderState("NoFrame", BlendStat[0]);
	Context->RegisterRenderState("Additive", BlendStat[1]);
	Context->RegisterRenderState("Blend", BlendStat[2]);
	Context->RegisterRenderState("AlphaBlend", BlendStat[3]);
	Context->RegisterRenderState("NoCull", RasterStat[0]);
	Context->RegisterRenderState("Rasterizer", RasterStat[1]);
}

int PrepassStage::Initial() {
	// create gbuffer abuffer first
	CreateGBuffer();
	CreateABuffer();
	CreateRenderState();
	return 0;
}

void PrepassStage::PrePass(RenderingCamera* Camera, Spatial* spatial, RenderQueue* renderQueue, WorkQueue* Queue, Vector<OsEvent*>& Events) {
	RenderView* renderview = RenderView::Create();
	RenderViews.PushBack(renderview);
	renderview->Camera = Camera;
	renderview->Depth = 0;
	renderview->Type = R_STAGE_PREPASSS;
	renderview->Index = 0;
	renderview->Queue = renderQueue;
	// set render target
	renderview->TargetCount = 5;
	renderview->Targets[0] = Targets[0];
	renderview->Targets[1] = CompactData[Frames % 2];
	renderview->Targets[2] = Targets[3];
	renderview->Targets[3] = Targets[4];
	renderview->Targets[4] = Targets[2];
	renderview->Depth = Depth;
	renderview->Parameters.Clear();
	renderview->ClearDepth = 1;
	renderview->ClearTargets = 1;
	// set compact previous data
	Variant variant;
	variant.as<int>() = CompactData[Frames % 2];
	Context->SetResource("gCompactBuffer", variant);
	variant.as<int>() = CompactData[(Frames + 1) % 2];
	Context->SetResource("gPrevCompactBuffer", variant);
	// 4. submit to workqueue
	int count = 1;
	while (count--) {
		CullingTask* task = CullingTask::Create();
		task->renderview = renderview;
		task->spatial = spatial;
		task->Context = Context;
		task->ObjectType = Node::RENDEROBJECT;
		Queue->QueueTask(task);
	}
	Events.PushBack(renderview->Event);
}

void PrepassStage::LigthingPass(RenderingCamera* Camera, Spatial* spatial, RenderQueue* renderQueue, WorkQueue* Queue, Vector<OsEvent*>& Events) {
	RenderView* renderview = RenderView::Create();
	RenderViews.PushBack(renderview);
	renderview->Camera = Camera;
	renderview->Depth = Depth;
	renderview->ClearDepth = 0;
	renderview->ClearTargets = 1;
	renderview->Type = R_STAGE_LIGHT;
	renderview->Index = 0;
	renderview->Queue = renderQueue;
	//renderview->Parameters.Clear();
	// set render target
	renderview->TargetCount = 1;
	renderview->Targets[0] = Context->GetRenderTarget("gPostBuffer"); // Targets[2];
	// 4. submit to workqueue
	int count = 1;
	while (count--) {
		CullingTask* task = CullingTask::Create();
		task->renderview = renderview;
		task->spatial = spatial;
		task->Context = Context;
		task->ObjectType = Node::LIGHT;
		Queue->QueueTask(task);
	}
	Events.PushBack(renderview->Event);
}

void PrepassStage::ShadingPass(RenderingCamera* Camera, Spatial* spatial, RenderQueue* renderQueue, WorkQueue* Queue, Vector<OsEvent*>& Events) {
	RenderView* renderview = RenderView::Create();
	RenderViews.PushBack(renderview);
	renderview->Camera = Camera;
	renderview->Type = R_STAGE_SHADING;
	renderview->Index = 0;
	renderview->Queue = renderQueue;
	renderview->Parameters.Clear();
	// set render target
	renderview->TargetCount = 1;
	renderview->Targets[0] = Context->GetRenderTarget("gPostBuffer");  // 0;
	renderview->Depth = Depth;
	renderview->ClearDepth = 0;
	renderview->ClearTargets = 1;
	// 4. submit to workqueue
	int count = 1;
	while (count--) {
		CullingTask* task = CullingTask::Create();
		task->renderview = renderview;
		task->spatial = spatial;
		task->Context = Context;
		task->ObjectType = Node::RENDEROBJECT;
		Queue->QueueTask(task);
	}
	Events.PushBack(renderview->Event);
}

void PrepassStage::OITInitPass(RenderingCamera* Camera, Spatial* spatial, RenderQueue* renderQueue, WorkQueue* Queue, Vector<OsEvent*>& Events) {
	RenderView* renderview = RenderView::Create();
	RenderViews.PushBack(renderview);
	renderview->Camera = Camera;
	renderview->Type = R_STAGE_OIT;
	renderview->Index = 0;
	renderview->Queue = renderQueue;
	renderview->Parameters.Clear();
	// set render target
	renderview->TargetCount = 0;
	renderview->Targets[0] = -1; // Context->GetRenderTarget("gPostBuffer");
	renderview->Depth = Depth;
	renderview->ClearDepth = 0;
	renderview->ClearTargets = 0;
	// 4. submit to workqueue
	int count = 1;
	while (count--) {
		CullingTask* task = CullingTask::Create();
		task->renderview = renderview;
		task->spatial = spatial;
		task->Context = Context;
		task->ObjectType = Node::TRANS;
		Queue->QueueTask(task);
	}
	Events.PushBack(renderview->Event);
}

int PrepassStage::Execute(RenderingCamera* Camera, Spatial* spatial, RenderQueue* renderQueue, WorkQueue* Queue, Vector<OsEvent*>& Events) {
	PrePass(Camera, spatial, renderQueue, Queue, Events);
	LigthingPass(Camera, spatial, renderQueue, Queue, Events);
	OITInitPass(Camera, spatial, renderQueue, Queue, Events);
	//	ShadingPass(Camera, spatial, renderQueue, Queue, Events);
	Frames++;
	return 0;
}

int PrepassStage::End() {
	// free renderviews
	RenderView* view = 0;
	int Size = RenderViews.Size();
	while (Size--) {
		view = RenderViews.PopBack();
		view->Recycle();
	}
	return 0;
}
