#include "D3d12Resource.h"
#include "DDSTextureLoader12.h"

using namespace D3D12Renderer;
using namespace DirectX;
/*
	create committed resource helper
*/
ID3D12Resource* CreateCommittedResource(ID3D12Device* d3d12Device, _In_  const D3D12_HEAP_PROPERTIES* pHeapProperties,
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
	// TODO: DXMA. 
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

void TextureResource::upload(ID3D12Device* d3d12Device, void* cpuData, unsigned int size)
{
	std::vector<D3D12_SUBRESOURCE_DATA > subresources;
	HRESULT result = LoadDDSTextureFromMemory(d3d12Device, (uint8_t*)cpuData, size, &resource, subresources, 0, NULL, &isCube);
	// get resource size
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(resource, 0, static_cast<UINT>(subresources.size()));
	// upload data to resource
	auto uploadHeap = UploadHeap::Alloc(d3d12Device, size);
	auto copyContext = D3D12CommandContext::Alloc(d3d12Device, D3D12_COMMAND_LIST_TYPE_COPY);
	auto cmdList = copyContext->GetCmdList();
	UpdateSubresources(cmdList, resource, uploadHeap->Get(),
		0, 0, static_cast<UINT>(subresources.size()), subresources.data());
	// resource barrier
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource,
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	// flush cmdList and wait for it
	copyContext->Flush(true);
	// safe to release resource
	uploadHeap->Release();
	copyContext->Release();
	// setdebugname
	resource->SetName(L"dds-texture");
}


void TextureResource::Create(ID3D12Device* d3d12Device, ResourceDescribe* resourceDesc)
{
	auto& textureDesc = resourceDesc->textureDesc;
	if (textureDesc.CpuData && textureDesc.Size) {
		// create texture from dds data
		upload(d3d12Device, textureDesc.CpuData, textureDesc.Size);
	} else {
		// create texture from description
		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Format = (DXGI_FORMAT)textureDesc.Format;
		resourceDesc.Width = textureDesc.Width;
		resourceDesc.Height = textureDesc.Height;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDesc.MipLevels = textureDesc.MipLevels;

		D3D12_RESOURCE_STATES createState = D3D12_RESOURCE_STATE_COPY_DEST;
		D3D12_CLEAR_VALUE clear = {};
		D3D12_CLEAR_VALUE* pClear = &clear;

		clear.Format = resourceDesc.Format;
		if (textureDesc.BindFlag & BIND_DEPTH_STENCIL) {
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			createState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
			// clearvalue
			clear.DepthStencil.Depth = 1;
			clear.DepthStencil.Stencil = 0;
			if (clear.Format == FORMAT_R32_TYPELESS) {
				clear.Format = (DXGI_FORMAT)FORMAT_D32_FLOAT;
			}
		} else if (textureDesc.BindFlag & BIND_RENDER_TARGET) {
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			createState = D3D12_RESOURCE_STATE_RENDER_TARGET;
			// clearvalue
			if (clear.Format == FORMAT_R32_TYPELESS) {
				clear.Format = (DXGI_FORMAT)FORMAT_R32_FLOAT;
			}
			clear.Color[0] = 0.0f;
			clear.Color[1] = 0.0f;
			clear.Color[2] = 0.0f;
			clear.Color[3] = 0.0f;
		}
		// uav
		if (textureDesc.BindFlag & BIND_UNORDERED_ACCESS) {
			// Num = NUM_FRAMES;
			resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			createState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			if (!(textureDesc.BindFlag & BIND_RENDER_TARGET)) {
				// not bind as renter target
				pClear = NULL;
			}
		}
		resource = CreateCommittedResource(
			d3d12Device,
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			createState,
			pClear);
		// set resource current state to createState
		state = createState;
		// set debug name
		resource->SetName(textureDesc.DebugName);
	}
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
	resource = CreateCommittedResource(d3d12Device,
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