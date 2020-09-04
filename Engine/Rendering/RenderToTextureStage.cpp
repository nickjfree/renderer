#include "RenderToTextureStage.h"
#include "Terrain.h"



void RenderToTextureStage::Initialize()
{
	CreateTerrainTextures();
}

// create terrain virtual textures
void RenderToTextureStage::CreateTerrainTextures()
{
	// virtual texture
	R_TEXTURE2D_DESC desc{};
	desc.Width = VirtualTextureSize;
	desc.Height = VirtualTextureSize;
	desc.ArraySize = 1;
	desc.CPUAccess = (R_CPU_ACCESS)0;
	desc.BindFlag = (R_BIND_FLAG)(BIND_RENDER_TARGET | BIND_SHADER_RESOURCE);
	desc.MipLevels = 1;
	desc.Usage = DEFAULT;
	desc.Format = FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.SampleDesc.Count = 1;
	terrainVirtualTexture_ = Interface->CreateTexture2D(&desc, nullptr, 0, 0);
	// page table
	R_BUFFER_DESC buffDesc = {};
	buffDesc.BindFlags = (R_BIND_FLAG)(BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS);
	buffDesc.CPUAccessFlags = (R_CPU_ACCESS)0;
	buffDesc.MiscFlags = (R_MISC)0;
	buffDesc.Size = VirtaulTexturePageTableSize;
	buffDesc.StructureByteStride = 4;
	buffDesc.Usage = DEFAULT;
	buffDesc.CPUData = nullptr;
	terrainPageTable_ = Interface->CreateBuffer(&buffDesc);
}

int RenderToTextureStage::TerrainVTPass(RenderingCamera* Camera, Spatial* spatial, RenderQueue* renderQueue, WorkQueue* Queue, Vector<OsEvent*>& Events)
{
	return 0;
}

RenderToTextureStage::RenderToTextureStage(RenderContext* context) : RenderStage(context) {
	Initialize();
}


RenderToTextureStage::~RenderToTextureStage()
{
}


int RenderToTextureStage::RenderTerrainTexture(RenderingCamera* Camera, Spatial* spatial, RenderQueue* renderQueue, WorkQueue* Queue, Vector<OsEvent*>& Events)
{
	Frustum frustum = Camera->GetFrustum();
	terrainNodes_.Reset();
	spatial->Query(frustum, terrainNodes_, Node::TERRAIN);

	// printf("terrain node %d\n", terrainNodes_.Size());
	// 
	int compiled = 0;

	// draw debug texture to backbuffer
	RenderView* renderview = RenderView::Create();
	// clear params
	renderview->Parameters.Clear();
	renderview->TargetCount = 1;
	renderview->ClearTargets = 0;
	// render to backbuffer
	renderview->Targets[0] = 0;
	renderview->Depth = -1;
	renderview->ClearDepth = 0;
	renderview->Type = R_STAGE_DEBUG;
	renderview->Camera = Camera;
	renderview->Queue = renderQueue;
	auto compiler = renderview->Compiler;
	char* buffer = (char*)renderview->CommandBuffer;
	compiler->SetBuffer(buffer);
	renderview->Compile(Context);
	// render all terrain nodes
	for (auto iter = terrainNodes_.Begin(); iter != terrainNodes_.End(); iter++) {
		auto node = *iter;
		compiled += node->Compile(compiler, renderview->Type, 0, renderview->Parameters, Camera, Context);
	}
	//compiler->Present();
	renderview->QueueCommand();
	// rememebr renderview
	renderViews_.PushBack(renderview);
	
	// update the page table
	if (terrainNodes_.Size()) {
		// update pagetable
		auto terrain = static_cast<TerrainNode*>(terrainNodes_[0])->GetTerrain();
	}
	return 0;
}


int RenderToTextureStage::Execute(RenderingCamera* Camera, Spatial* spatial, RenderQueue* renderQueue, WorkQueue* Queue, Vector<OsEvent*>& Events)
{
	auto compiled = RenderTerrainTexture(Camera, spatial, renderQueue, Queue, Events);
	return 0;
}

int RenderToTextureStage::End()
{
	int Size = renderViews_.Size();
	while (Size--) {
		auto view = renderViews_.PopBack();
		view->Recycle();
	}
	return 0;
}
