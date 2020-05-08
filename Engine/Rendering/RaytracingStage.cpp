
#include "RaytracingStage.h"

void RaytracingStage::Initialize()
{
	int Width = Context->FrameWidth;
	int Height = Context->FrameHeight;
	R_TEXTURE2D_DESC desc = {};
	desc.Width = Width;
	desc.Height = Height;
	desc.ArraySize = 1;
	desc.CPUAccess = (R_CPU_ACCESS)0;
	desc.BindFlag = (R_BIND_FLAG)(BIND_UNORDERED_ACCESS | BIND_SHADER_RESOURCE);
	desc.MipLevels = 1;
	desc.Usage = DEFAULT;
	desc.Format = FORMAT_R16G16B16A16_FLOAT;
	desc.SampleDesc.Count = 1;

	rtTarget = Interface->CreateTexture2D(&desc, 0, 0, 0);
}


RaytracingStage::RaytracingStage(RenderContext* Context): RenderStage(Context)
{
	Initialize();
}

RaytracingStage::~RaytracingStage()
{
}

int RaytracingStage::Execute(RenderingCamera* Camera, Spatial* spatial, RenderQueue* renderQueue, WorkQueue* Queue, Vector<OsEvent*>& Events)
{
	// get all the renderobjs
	rtInstances.Empty();
	spatial->Query(rtInstances, Node::RENDEROBJECT);
	int Size = rtInstances.Size();
	// add rtInstance to Scene
	for (auto Iter = rtInstances.Begin(); Iter != rtInstances.End(); Iter++) {
		auto renderObject = *Iter;
		//renderObject->Compile();
		renderObject->UpdateRaytracingStructure(Context);
	}
	// let's shoot rays
	if (TestShader) {
		//Compiled += Compiler->SetDepthBuffer(-1);
		auto renderview = RenderView::Create();
		renderview->Camera = Camera;
		renderview->Depth = -1;
		renderview->Type = R_STAGE_RT;
		renderview->Index = 0;
		renderview->Queue = renderQueue;
		renderview->TargetCount = 0;

		auto compiler = renderview->Compiler;
		compiler->SetBuffer((char*)renderview->CommandBuffer);

		auto compiled = 0;
		compiled += compiler->SetDepthBuffer(-1);

		Parameter["RenderTarget"].as<int>() = rtTarget;
		Parameter["g_rayGenCB"].as<RayGenConstantBuffer>() = rayGenCB;

		compiled += TestShader->Compile(compiler, 0, 0, Parameter, Parameter, Context);
		compiled += compiler->TraceRay();

		renderview->QueueCommand();

		RenderViews.PushBack(renderview);

	} else {
		Variant* Value = Context->GetResource("ShaderLibrary\\shaders\\test.cso");
		if (Value) {
			TestShader = Value->as<ShaderLibrary*>();
		}
	}
	return 0;
}

int RaytracingStage::End()
{
	int Size = RenderViews.Size();
	while (Size--) {
		auto view = RenderViews.PopBack();
		view->Recycle();
	}
	return 0;
}
