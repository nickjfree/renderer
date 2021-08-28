#include "RenderControl.h"
#include "CullingTask.h"
#include "PrepassStage.h"
#include "PostpassStage.h"
#include "ShadowMapStage.h"
#include "RaytracingStage.h"
#include "RenderToTextureStage.h"
#include "Opcode.h"
#include "FrameGraph.h"

#include "RenderPassShadowMap.h"
#include "RenderPassRaster.h"
#include "RenderPassHDR.h"
#include "RenderPassSSAO.h"
#include "RenderPassRaytracing.h"
#include "RenderPassFSR.h"
#include "RenderPassGI.h"


RenderControl::RenderControl(RenderContext* Context_) :Context(Context_)
{
	memset(RenderPath, 0, MAX_RENDER_PATH * sizeof(void*));
}


RenderControl::~RenderControl()
{
}


int RenderControl::Initialize() {
	//RenderQueue_ = new RenderQueue();
	//// init render processer
	//RenderProcesser_ = new RenderProcesser(Context);
	//RenderingPath* LightPrePass = new RenderingPath(Context);
	//RenderStage* Stage = 0;
	//// initialize shadow map pass
	//Stage = new ShadowMapStage(Context);
	//LightPrePass->Stages.PushBack(Stage);
	//// initialize rtt pass
	//Stage = new RenderToTextureStage(Context);
	//LightPrePass->Stages.PushBack(Stage);
	//// initialize light-pre pass
	//Stage = new PrepassStage(Context);
	//LightPrePass->Stages.PushBack(Stage);
	//// initialize post pass
	//Stage = new PostpassStage(Context);
	//LightPrePass->Stages.PushBack(Stage);
	//// initialize raytracing stage
	//Stage = new RaytracingStage(Context);
	//LightPrePass->Stages.PushBack(Stage);
	//RenderPath[LIGHT_PRE] = LightPrePass;

	initFrameGraph();
	return 0;
}
//
//int RenderControl::Execute() {
//
//	// profile
//	PIXScopedEvent(0xFFFFFF00, __FUNCTION__);
//
//	int numCamera = Cameras.Size();
//
//	if (!numCamera) {
//		printf("no camera\n");
//		char cmd[] = { OP_RENDER_TARGET, 1, 0, 0, 0, 0, OP_PRESENT, OP_END_EXECUTE };
//		RenderQueue_->PushCommand(R_STAGE_PRESENT, cmd);
//		RenderQueue_->Execute(RenderProcesser_);
//		return 1;
//	}
//
//	while (numCamera--) {
//		RenderingCamera* cam = Cameras[numCamera];
//		startCamera(cam);
//	}
//
//	// wait for tasks finish
//	int Count = Events.Size();
//	OsEvent::Join(Count, &Events[0], 1);
//	Events.Reset();
//	RenderQueue_->Execute(RenderProcesser_);
//	// end stages
//	RenderingPath* path = RenderPath[LIGHT_PRE];
//	int stages = path->Stages.Size();
//	for (int i = 0; i < stages; i++) {
//		RenderStage* Stage = path->Stages[i];
//		Stage->End();
//	} 
//	return 0;
//}

int RenderControl::Execute() {

	// profile
	PIXScopedEvent(0xFFFFFF00, __FUNCTION__);

	int numCamera = Cameras.Size();

	if (!numCamera) {
		//printf("no camera\n");
		return 1;
	}

	while (numCamera--) {
		RenderingCamera* cam = Cameras[numCamera];
		// render the frame
		cam->Update(Context);
		frameGraph.Execute(cam, spatial, Context);
		// update prev matrix. for motion vectors
		cam->UpdatePrevMatrix();
	}
	return 0;
}

void RenderControl::startCamera(RenderingCamera* Camera) {
	// light pre pass only
	RenderingPath* path = RenderPath[LIGHT_PRE];
	int stages = path->Stages.Size();
	for (int i = 0; i < stages; i++) {
		RenderStage* Stage = path->Stages[i];
		Stage->Execute(Camera, spatial, RenderQueue_, Queue, Events);
	}
	// update prev matrix. for motion vectors
	Camera->UpdatePrevMatrix();
}

int RenderControl::AddCamera(RenderingCamera* Camera) {
	float pi = 3.141592654f;
	Matrix4x4 Projection = Matrix4x4::PerspectiveFovLH(0.3f * pi, (float)FRAMEBUFFER_WIDTH / FRAMEBUFFER_HEIGHT, 1, 1000);
	Camera->SetProjection(Projection);
	Cameras.PushBack(Camera);
	return 0;
}

int RenderControl::RemoveCamera(RenderingCamera* Camera) {
	for (int i = 0; i < Cameras.Size(); i++) {
		if (Cameras[i] == Camera) {
			for (auto j = i; j < Cameras.Size() - 1; j++) {
				Cameras[j] = Cameras[j + 1];
			}
			Cameras.PopBack();
		}
	}
	return 0;
}


void RenderControl::initFrameGraph()
{
	// create all render state
	CreateRenderState(Context);
	// build the frame graph
	auto gbuffer = AddGBufferPass(frameGraph, Context);
	auto shadow = AddShadowMapPass(frameGraph, Context);
	auto lighting = AddLightingPass(frameGraph, Context, gbuffer->Data());
	auto emissive = AddEmissivePass(frameGraph, Context, gbuffer->Data(), lighting->Data());
	auto ssao = AddSSAOPass(frameGraph, Context, lighting->Data(), gbuffer->Data());
	auto lightculling = AddLightCullingPass(frameGraph, Context);
	// gi
	auto gi = AddGIPass(frameGraph, Context, lightculling->Data());
	// rt-lighting
	auto rtLighting = AddRaytracedLightingPass(frameGraph, Context, gbuffer->Data(), lightculling->Data(), gi->Data());
	// rt-relection
	auto relection = AddRaytracedReflectionPass(frameGraph, Context, gbuffer->Data(), lightculling->Data(), gi->Data());
	// resolve
	auto resolved = AddResolvePass(frameGraph, Context, gbuffer->Data(), lighting->Data(), ssao->Data(), relection->Data(), rtLighting->Data());
	auto hdr = AddHDRPass(frameGraph, Context, resolved->Data());
	auto fsr = AddFSRPass(frameGraph, Context, hdr->Data());
	// build as
	auto as = AddBuildASPass(frameGraph, Context);
}
