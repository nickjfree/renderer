#include "D3D12Renderer.h"
#include "D3D12Resource.h"
#include "d3dx12.h"

using namespace D3D12Renderer;



D3D12Resource* D3D12RenderInterface::GetResource(int id)
{
	auto resourceType = id & 0xff000000 >> 24;
	D3D12Resource* resource = nullptr;
	switch (resourceType) {
	case static_cast<int>(D3D12Resource::RESOURCE_TYPES::TEXTURE):
		resource = TextureResource::Get(id);
		break;
	case static_cast<int>(D3D12Resource::RESOURCE_TYPES::BUFFER):
		resource = BufferResource::Get(id);
		break;
	case static_cast<int>(D3D12Resource::RESOURCE_TYPES::GEOMETRY):
		resource = Geometry::Get(id);
		break;
	}
	return resource;
}

// destory resource
void D3D12RenderInterface::DestoryResource(int id)
{
	auto resource = GetResource(id);
	resource->Release();
}

int D3D12RenderInterface::CreateTexture2D(R_TEXTURE2D_DESC* desc)
{
	// alloc resource index
	auto texture = TextureResource::Alloc();
	// TODO:
	// create the texture resource
	return texture->resourceId;
}

int D3D12RenderInterface::DestoryTexture2D(int id)
{
	return 0;
}

int D3D12RenderInterface::CreateBuffer(R_BUFFER_DESC* desc)
{
	// alloc resource index
	auto buffer = BufferResource::Alloc();
	// TODO:
	// create the buffer resource
	return buffer->resourceId;
}


int D3D12RenderInterface::DestoryBuffer(int id)
{
	return 0;
}

