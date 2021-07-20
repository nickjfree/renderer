
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
	desc.DebugName = L"rt-target";
	rtTarget = Interface->CreateTexture2D(&desc, 0, 0, 0);
	desc.DebugName = L"rt-target-test";
	rtTestTarget = Interface->CreateTexture2D(&desc, 0, 0, 0);


	desc.BindFlag = (R_BIND_FLAG)(BIND_RENDER_TARGET | BIND_SHADER_RESOURCE);
	desc.DebugName = L"svgf-color-0";
	AccColor[0] = Interface->CreateTexture2D(&desc, 0, 0, 0);
	desc.DebugName = L"svgf-color-1";
	AccColor[1] = Interface->CreateTexture2D(&desc, 0, 0, 0);
	desc.Format = FORMAT_R16G16B16A16_FLOAT;
	desc.DebugName = L"svgf-moments-0";
	AccMoments[0] = Interface->CreateTexture2D(&desc, 0, 0, 0);
	desc.DebugName = L"svgf-moments-1";
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

	Parameter["gPrevColor"] = AccColor[(NumFrames + 1) % 2];
	// Parameter["gPrevColor"] = AccColor[1];
	Parameter["gPrevMoment"] = AccMoments[(NumFrames + 1) % 2];
	Parameter["gCurrentColor"] = rtTarget;
	compiled += DenosingShader->Compile(Compiler, 0, 0, Parameter, Parameter, Context);

	int Targets[2] = { AccColor[NumFrames % 2],  AccMoments[NumFrames % 2] };
	// int Targets[2] = { AccColor[0],  AccMoments[NumFrames % 2] };

	compiled += Compiler->SetRenderTargets(2, Targets);
	compiled += Compiler->Quad();
	// set result color as gRaytracingBuffer
	/*Variant variant;
	variant= Targets[0];
	Context->SetResource("gRaytracingBuffer", variant);*/
	return compiled;
}


int RaytracingStage::CalcVariance(BatchCompiler* Compiler) 
{

	/**
		Texture2D gColor : register(t0);
		Texture2D gMoment : register(t1);
	*/
	auto compiled = 0;
	Parameter["gColor"] = AccColor[NumFrames % 2];
	Parameter["gMoment"] = AccMoments[NumFrames % 2];

	int target = AccColor[(NumFrames + 1) % 2];
	// filter
	compiled += DenosingShader->Compile(Compiler, 2, 0, Parameter, Parameter, Context);

	compiled += Compiler->SetRenderTargets(1, &target);
	compiled += Compiler->Quad();
	return compiled; 
}


int RaytracingStage::Filter(BatchCompiler* Compiler) {

	auto compiled = 0;
	Parameter["gColor"] = AccColor[(NumFrames + 1) % 2];
	// Parameter["gColor"] = AccColor[0];
	Parameter["gMoment"] = AccMoments[NumFrames % 2];

	int target = AccColor[NumFrames % 2];
	// int target = AccColor[1];
	// filter
	compiled += DenosingShader->Compile(Compiler, 1, 0, Parameter, Parameter, Context);
	
	compiled += Compiler->SetRenderTargets(1, &target);
	compiled += Compiler->Quad();

	// set result
	Variant variant;
	variant= target;
	Context->SetResource("gRaytracingBuffer", variant);

	return compiled;
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
		compiled += CalcVariance(Compiler);
		// filter color
		compiled += Filter(Compiler);
	}
	return compiled;
}

int RaytracingStage::BuildRaytracingScene(RenderingCamera* Camera, Spatial* spatial, BatchCompiler* compiler)
{
	auto compiled = 0;
	// get all the renderobjs
	rtInstances.Reset();
	spatial->Query(rtInstances, Node::RENDEROBJECT|Node::LIGHT);
	int Size = rtInstances.Size();
	// add rtInstance to Scene
	for (auto Iter = rtInstances.Begin(); Iter != rtInstances.End(); Iter++) {
		auto renderObject = *Iter;
		//renderObject->Compile();
		// renderObject->UpdateRaytracingStructure(Context);
	}
	auto Value = Context->GetResource("Material\\Materials\\reflection.xml\\0");
	if (Value) {
		compiler->BuildRaytracingScene();
	}
	return compiled;
}

int RaytracingStage::BuildLightingData(RenderingCamera* Camera, Spatial* spatial, BatchCompiler* compiler) 
{
	// clear prev data
	lights.Reset();
	lightingData.Reset();
	spatial->Query(Camera->GetFrustum(), lights, Node::LIGHT);

	for (auto iter = lights.Begin(); iter != lights.End(); iter++) {
		RenderLight* light = (RenderLight*)*iter;
		lightingData.PushBack(light->GetLightData());
	}
	ShaderParameterArray shaderParameterLights { lightingData.GetData(), lightingData.Size() * sizeof(LightData) };
	Parameter["gLightArray"] = shaderParameterLights;
	return 0;
}


int RaytracingStage::Raytracing(RenderingCamera* Camera, Spatial* spatial, BatchCompiler* compiler)
{

	auto compiled = 0;
	// let's shoot rays
	auto Value = Context->GetResource("Material\\Materials\\reflection.xml\\0");
	if (Value) {
		auto material = Value->as<Material*>();
		auto rtShader = material->GetShaderLibrary(0);
		//Compiled += Compiler->SetDepthBuffer(-1);
		// build raytracing scene in compute queue
		// compiler->BuildRaytracingScene();
		// compiled += compiler->SetDepthBuffer(-1);

		// Parameter["RenderTarget"] = Context->GetRenderTarget("gPostBuffer"); // rtTarget
		Parameter["RenderTarget"] = rtTarget;
		//Parameter["PrevRenderTarget"] = rtTarget[(NumFrames + 1) % 2];
		// Context->SetResource("rtTarget", Parameter["RenderTarget"]);

		Matrix4x4::Tranpose(Camera->GetInvertView(), &Parameter["gInvertViewMaxtrix"].as<Matrix4x4>());
		Matrix4x4::Tranpose(Camera->GetViewProjection(), &Parameter["gViewProjectionMatrix"].as<Matrix4x4>());
		Matrix4x4::Tranpose(Camera->GetViewMatrix(), &Parameter["gViewMatrix"].as<Matrix4x4>());
		Parameter["gViewPoint"] = Camera->GetViewPoint();
		Parameter["gScreenSize"] = Vector2(static_cast<float>(Context->FrameWidth), static_cast<float>(Context->FrameHeight));
		Parameter["gFrameNumber"] = NumFrames;

		compiled += rtShader->Compile(compiler, 0, 0, material->GetParameter(), Parameter, Context);
		compiled += compiler->TraceRay();
		// test run
		//Parameter["RenderTarget"] = rtTestTarget;
		//compiled += rtShader->Compile(compiler, 0, 0, material->GetParameter(), Parameter, Context);
		//compiled += compiler->TraceRay();
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
		// build lights
		compiled += BuildLightingData(Camera, spatial, compiler);
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
