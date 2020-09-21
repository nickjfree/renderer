
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


	desc.BindFlag = (R_BIND_FLAG)(BIND_RENDER_TARGET | BIND_SHADER_RESOURCE);

	AccColor[0] = Interface->CreateTexture2D(&desc, 0, 0, 0);
	AccColor[1] = Interface->CreateTexture2D(&desc, 0, 0, 0);
	desc.Format = FORMAT_R16G16B16A16_FLOAT;
	AccMoments[0] = Interface->CreateTexture2D(&desc, 0, 0, 0);
	AccMoments[1] = Interface->CreateTexture2D(&desc, 0, 0, 0);

	// rtTarget[1] = Interface->CreateTexture2D(&desc, 0, 0, 0);
	Context->RegisterRenderTarget("gRtTarget", rtTarget);
}


RaytracingStage::RaytracingStage(RenderContext* Context): RenderStage(Context)
{
	Initialize();
}

RaytracingStage::~RaytracingStage()
{
}

int RaytracingStage::Accumulation(BatchCompiler* Compiler) 
{
	/*
		Texture2D gPrevColor : register(t0);
		Texture2D gPrevMoment : register(t1);
		Texture2D gCurrentColor : register(t2);
	*/
	auto compiled = 0;

	Parameter["gPrevColor"].as<int>() = AccColor[1];
	Parameter["gPrevMoment"].as<int>() = AccMoments[(NumFrames + 1) % 2];
	Parameter["gCurrentColor"].as<int>() = rtTarget;
	compiled += DenosingShader->Compile(Compiler, 0, 0, Parameter, Parameter, Context);

	int Targets[2] = { AccColor[0],  AccMoments[NumFrames % 2] };
	compiled += Compiler->SetRenderTargets(2, Targets);
	compiled += Compiler->Quad();
	// set result color as gRaytracingBuffer
	/*Variant variant;
	variant.as<int>() = Targets[0];
	Context->SetResource("gRaytracingBuffer", variant);*/
	return compiled;
}

int RaytracingStage::Filter(BatchCompiler* Compiler) {

	auto compiled = 0;
	Parameter["gColor"].as<int>() = AccColor[0];
	Parameter["gMoment"].as<int>() = AccMoments[NumFrames % 2];

	int Target = AccColor[1];
	// filter
	compiled += DenosingShader->Compile(Compiler, 1, 0, Parameter, Parameter, Context);
	
	compiled += Compiler->SetRenderTargets(1, &Target);
	compiled += Compiler->Quad();

	Variant variant;
	variant.as<int>() = Target;
	Context->SetResource("gRaytracingBuffer", variant);

	return 0;
}

int RaytracingStage::Denosing(BatchCompiler* Compiler) {
	auto compiled = 0;
	// get shaders
	if (!DenosingShader) {
		auto Value = Context->GetResource("Shader\\shaders\\Denoising\\0");
		if (Value) {
			DenosingShader = Value->as<Shader*>();
		}
	} else {
		// shader loaded, we can do the denosing
		compiled += Accumulation(Compiler);
		// filter moments
		compiled += Filter(Compiler);
		// 
	}
	return compiled;
}

int RaytracingStage::BuildRaytracingScene(RenderingCamera* Camera, Spatial* spatial, BatchCompiler* compiler)
{
	auto compiled = 0;
	// get all the renderobjs
	rtInstances.Reset();
	spatial->Query(rtInstances, Node::RENDEROBJECT);
	int Size = rtInstances.Size();
	// add rtInstance to Scene
	for (auto Iter = rtInstances.Begin(); Iter != rtInstances.End(); Iter++) {
		auto renderObject = *Iter;
		//renderObject->Compile();
		renderObject->UpdateRaytracingStructure(Context);
	}
	auto Value = Context->GetResource("Material\\Materials\\reflection.xml\\0");
	if (Value) {
		compiler->BuildRaytracingScene();
	}
	return compiled;
}


int RaytracingStage::Raytracing(RenderingCamera* Camera, Spatial* spatial, BatchCompiler* compiler)
{

	auto compiled = 0;
	// let's shoot rays
	auto Value = Context->GetResource("Material\\Materials\\reflection.xml\\0");
	if (Value) {
		auto rtShader = Value->as<Material*>()->GetShaderLibrary();
		//Compiled += Compiler->SetDepthBuffer(-1);
		// build raytracing scene in compute queue
		// compiler->BuildRaytracingScene();
		// compiled += compiler->SetDepthBuffer(-1);

		// Parameter["RenderTarget"].as<int>() = Context->GetRenderTarget("gPostBuffer"); // rtTarget
		Parameter["RenderTarget"].as<int>() = rtTarget;
		//Parameter["PrevRenderTarget"].as<int>() = rtTarget[(NumFrames + 1) % 2];
		Context->SetResource("rtTarget", Parameter["RenderTarget"]);

		Matrix4x4::Tranpose(Camera->GetInvertView(), &Parameter["gInvertViewMaxtrix"].as<Matrix4x4>());
		Parameter["gViewPoint"].as<Vector3>() = Camera->GetViewPoint();
		Parameter["gScreenSize"].as<Vector2>() = Vector2(static_cast<float>(Context->FrameWidth), static_cast<float>(Context->FrameHeight));
		Parameter["gFrameNumber"].as<int>() = NumFrames;

		compiled += rtShader->Compile(compiler, 0, 0, Parameter, Parameter, Context);
		compiled += compiler->TraceRay();
	}
	return compiled;
}

int RaytracingStage::Execute(RenderingCamera* Camera, Spatial* spatial, RenderQueue* renderQueue, WorkQueue* Queue, Vector<OsEvent*>& Events) 
{
	{
		auto compiled = 0;
		// create a renderview
		auto renderview = RenderView::Create();
		renderview->Camera = Camera;
		renderview->Depth = -1;
		renderview->Type = R_STAGE_RT_BUILD;
		renderview->Index = 0;
		renderview->Queue = renderQueue;
		renderview->TargetCount = 0;

		auto compiler = renderview->Compiler;
		compiler->Reset();
		// build scene
		BuildRaytracingScene(Camera, spatial, compiler);
		// recycle
		renderview->QueueCommand();
		RenderViews.PushBack(renderview);
	}
	// trace ray
	{
		auto compiled = 0;
		// create a renderview
		auto renderview = RenderView::Create();
		renderview->Camera = Camera;
		renderview->Depth = -1;
		renderview->Type = R_STAGE_RT_DISPATCH2;
		renderview->Index = 0;
		renderview->Queue = renderQueue;
		renderview->TargetCount = 0;

		auto compiler = renderview->Compiler;
		compiler->Reset();
		// trace rays
		compiled += Raytracing(Camera, spatial, compiler);
		// denoising
		compiled += Denosing(compiler);

		// recycle
		renderview->QueueCommand();
		RenderViews.PushBack(renderview);
	}
	// inc  frames
	NumFrames++;
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
