#include "ShadowMapStage.h"
#include "RenderLight.h"
#include "CullingTask.h"



ShadowMapStage::ShadowMapStage(RenderContext* Context) : RenderStage(Context) {
	Initialize();
}


ShadowMapStage::~ShadowMapStage() {
}

void ShadowMapStage::CreateShadowMaps() {
	R_TEXTURE2D_DESC desc = {};
	desc.Width = Context->FrameWidth;
	desc.Height = Context->FrameHeight;
	desc.ArraySize = 1;
	desc.CPUAccess = (R_CPU_ACCESS)0;
	desc.BindFlag = (R_BIND_FLAG)(BIND_DEPTH_STENCIL | BIND_SHADER_RESOURCE);
	desc.MipLevels = 1;
	desc.Usage = DEFAULT;
	desc.SampleDesc.Count = 1;
	desc.Format = FORMAT_R32_TYPELESS;
	desc.DebugName = L"shadowmap";
	for (int i = 0; i < SHADOWSTAGE_SHADOWMAPS; i++) {
		Targets[i] = Interface->CreateTexture2D(&desc, NULL, 0, 0);
	}
}


void ShadowMapStage::Initialize() {
	CreateShadowMaps();
}


int ShadowMapStage::ShadowPass(RenderingCamera* Camera, Spatial* spatial, RenderQueue* renderQueue, WorkQueue* Queue, Vector<OsEvent*>& Events) {
	// find all affected lights and submit culling tasks for each light for shadow mapping
	Frustum frustum = Camera->GetFrustum();
	Lights.Reset();
	spatial->Query(frustum, Lights, Node::LIGHT);
	int Size = Lights.Size();
	int TargetsUsed = 0;
	for (int i = 0; i < Size && TargetsUsed < SHADOWSTAGE_SHADOWMAPS; i++) {
		RenderLight* Light = (RenderLight*)Lights[i];
		// just assum all lights are shadow casting lights for now
		// create render view with light cameras and submit tasks
		Light->SetShadowMap(Targets[TargetsUsed]);
		RenderView* renderview = RenderView::Create();
		RenderViews.PushBack(renderview);
		renderview->Camera = Light->GetLightCamera();
		renderview->Depth = Targets[TargetsUsed];
		renderview->Type = R_STAGE_SHADOW;
		renderview->Index = TargetsUsed++;
		renderview->Queue = renderQueue;
		//renderview->Parameters.Clear();
		// set render target
		renderview->TargetCount = 0;
		renderview->ClearDepth = 1;
		// 4. submit to workqueue 
		int count = 1;
		while (count--) {
			CullingTask* task = CullingTask::Create();
			task->renderview = renderview;
			task->spatial = spatial;
			task->ObjectType = Node::RENDEROBJECT;
			task->Context = Context;
			Queue->QueueTask(task);
		}
		Events.PushBack(renderview->Event);
	}
	return 0;
}


int ShadowMapStage::Execute(RenderingCamera* Camera, Spatial* spatial, RenderQueue* renderQueue, WorkQueue* Queue, Vector<OsEvent*>& Events) {
	ShadowPass(Camera, spatial, renderQueue, Queue, Events);
	return 0;
}

int ShadowMapStage::End() {
	// free renderviews
	RenderView* view = 0;
	int Size = RenderViews.Size();
	while (Size--) {
		view = RenderViews.PopBack();
		view->Recycle();
	}
	return 0;
}