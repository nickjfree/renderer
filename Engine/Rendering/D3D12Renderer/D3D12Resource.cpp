#include "D3d12Resource.h"


using namespace D3D12Renderer;

/*
	create commited resource helper
*/
ID3D12Resource* CreateCommitedResource(ID3D12Device* d3d12Device, _In_  const D3D12_HEAP_PROPERTIES* pHeapProperties,
	D3D12_HEAP_FLAGS HeapFlags,
	_In_  const D3D12_RESOURCE_DESC* pDesc,
	D3D12_RESOURCE_STATES InitialResourceState,
	_In_opt_  const D3D12_CLEAR_VALUE* pOptimizedClearValue)
{
#ifndef USE_DXMA
	// default. just create commited resource
	ID3D12Resource* ret = nullptr;
	HRESULT hResult = d3d12Device->CreateCommittedResource(pHeapProperties, HeapFlags, pDesc, InitialResourceState, pOptimizedClearValue, IID_PPV_ARGS(&ret));
	return ret;
#else

#endif
}


void BufferResource::Create(ID3D12Device* d3d12Device, ResourceDescribe* resourceDesc)
{
	auto& bufferDesc = resourceDesc->bufferDesc;
}

void BufferResource::Release()
{
	Free();
}

void TextureResource::Create(ID3D12Device* d3d12Device, ResourceDescribe* resourceDesc)
{
	auto& textureDesc = resourceDesc->textureDesc;
}

void TextureResource::Release()
{
	Free();
}

void Geometry::Create(ID3D12Device* d3d12Device, ResourceDescribe* resourceDesc)
{
}

void Geometry::Release()
{
	Free();
}

UploadHeap* UploadHeap::Alloc(ID3D12Device* d3d12Device, unsigned int size)
{
	auto uploadHeap = new UploadHeap();
	uploadHeap->create(d3d12Device, size);
	return uploadHeap;
}

UploadHeap* UploadHeap::AllocTransient(ID3D12Device* d3d12Device, unsigned int size)
{
	auto uploadHeap = allocTransient(
	[&](UploadHeap* uploadHeap) {
		uploadHeap->create(d3d12Device, size);
	},
	[&](UploadHeap* uploadHeap) {
		return true;
	});
	return uploadHeap;
}

void UploadHeap::create(ID3D12Device* d3d12Device, unsigned int size)
{
	D3D12_HEAP_TYPE heapType{};
	resource = CreateCommitedResource(d3d12Device,
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(size, D3D12_RESOURCE_FLAG_NONE),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr);
}

void UploadHeap::Release()
{
	resetTransient();
	resource->Release();
	delete this;
}

bool UploadHeap::SubAlloc(unsigned int allocSize)
{

	int alignedSize = (allocSize + const_buffer_align - 1) & ~(const_buffer_align - 1);

	if (currentOffset + alignedSize > size) {
		// not enough space
		return false;
	}
	// 
	currentOffset += alignedSize;
	return true;
}

/*
*	arguments:
*		size:             buffer size to allocate
*		gpuAddress(out):  pointer to gpu address
	return:
		cpu address
*/
void* RingConstantBuffer::AllocTransientConstantBuffer(unsigned int size, void** gpuAddress)
{
	// suballoc constant buffer space
	if (!currentUploadHeap || !currentUploadHeap->SubAlloc(size)) {
		// alloc a new transient const buffer
		currentUploadHeap = UploadHeap::AllocTransient(d3d12Device, max_upload_heap_size);
	}
	*gpuAddress = (void*)currentUploadHeap->GetCurrentGpuVirtualAddress();
	return currentUploadHeap->GetCurrentCpuVirtualAddress();
}

void RingConstantBuffer::Reset()
{
	currentUploadHeap = nullptr;
}