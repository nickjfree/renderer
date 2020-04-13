
#include "RaytracingStage.h"

void RaytracingStage::Initialize()
{
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
	return 0;
}
