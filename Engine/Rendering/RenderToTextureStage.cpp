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
