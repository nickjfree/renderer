#include "D3d12Resource.h"
#include "D3D12Renderer.h"
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


/************************************************************************/
// D3D12Resource
/************************************************************************/

void D3D12Resource::SetResourceState(D3D12CommandContext* cmdContext, D3D12_RESOURCE_STATES targetState)
{
	if (state != targetState) {
		cmdContext->AddBarrier(CD3DX12_RESOURCE_BARRIER::Transition(resource, state, targetState));
	} else if (targetState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS && state == D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
		// the resource state is not changed, but it is a uav resource. this means the caller wants to issue a uav barrier
		cmdContext->AddBarrier(CD3DX12_RESOURCE_BARRIER::UAV(resource));
	}
	state = targetState;
}


void D3D12Resource::CreateViews(ID3D12Device* d3d12Device, ResourceDescribe* resourceDesc, D3D12DescriptorHeap** descHeaps)
{
}


/************************************************************************/
// BufferResource
/************************************************************************/

void BufferResource::Upload(ID3D12Device* d3d12Device, D3D12CommandContext* copyContext, UploadHeap* uploadHeap, void* cpuData, unsigned int size)
{
	// upload data to resource
	auto cmdList = copyContext->GetCmdList();

	D3D12_SUBRESOURCE_DATA Data = {};
	Data.pData = cpuData;
	Data.RowPitch = size;
	Data.SlicePitch = 1;
	UpdateSubresources(cmdList, resource, uploadHeap->Get(),
		0, 0, 1, &Data);
}

void BufferResource::Create(ID3D12Device* d3d12Device, ResourceDescribe* resourceDesc)
{
	auto& bufferDesc = resourceDesc->bufferDesc;
	// create buffer
	resource = CreateCommittedResource(
		d3d12Device,
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferDesc.Size, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr);
	state = D3D12_RESOURCE_STATE_COPY_DEST;
	if (bufferDesc.CPUData && bufferDesc.Size) {
		// alloc command context and uploadHeap
		auto uploadBufferSize = GetRequiredIntermediateSize(resource, 0, 1);
		auto copyContext = D3D12CommandContext::Alloc(d3d12Device, D3D12_COMMAND_LIST_TYPE_DIRECT);
		auto uploadHeap = UploadHeap::Alloc(d3d12Device, uploadBufferSize);
		// do the upload
		Upload(d3d12Device, copyContext, uploadHeap, bufferDesc.CPUData, bufferDesc.Size);
		// get resource barrier target state
		auto targetState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		if (bufferDesc.BindFlags == BIND_VERTEX_BUFFER) {
			targetState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER | D3D12_RESOURCE_STATE_COPY_SOURCE;
		} else if (bufferDesc.BindFlags == BIND_INDEX_BUFFER) {
			targetState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_INDEX_BUFFER | D3D12_RESOURCE_STATE_COPY_SOURCE;
		}
		// set current state
		SetResourceState(copyContext, targetState);
		// flush cmdList and wait for it
		copyContext->Flush(true);
		// safe to release resource
		uploadHeap->Release();
		copyContext->Release();
	}
	// set debug name
	resource->SetName(bufferDesc.DebugName);
}


void BufferResource::CreateViews(ID3D12Device* d3d12Device, ResourceDescribe* resourceDesc, D3D12DescriptorHeap** descHeaps)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC vdesc = {};
	D3D12_UNORDERED_ACCESS_VIEW_DESC udesc = {};
	auto& bufferDesc = resourceDesc->bufferDesc;
	// srv desc
	vdesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	vdesc.Format = (DXGI_FORMAT)FORMAT_UNKNOWN;
	vdesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	vdesc.Buffer.FirstElement = 0;
	vdesc.Buffer.NumElements = bufferDesc.Size / bufferDesc.StructureByteStride;
	vdesc.Buffer.StructureByteStride = bufferDesc.StructureByteStride;
	vdesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	// uav desc
	udesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	udesc.Format = (DXGI_FORMAT)FORMAT_UNKNOWN;
	udesc.Buffer.FirstElement = 0;
	udesc.Buffer.NumElements = bufferDesc.Size / bufferDesc.StructureByteStride;
	udesc.Buffer.StructureByteStride = bufferDesc.StructureByteStride;
	udesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	// srv
	auto constexpr srvIndex = (int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::SRV;
	auto handle = descHeaps[srvIndex]->GetCpuHandle(resourceId & 0x00ffffff);
	d3d12Device->CreateShaderResourceView(resource, &vdesc, handle);
	views[srvIndex] = handle;
	// uav
	auto constexpr uavIndex = (int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::UAV;
	handle = descHeaps[uavIndex]->GetCpuHandle(resourceId & 0x00ffffff);
	d3d12Device->CreateUnorderedAccessView(resource, nullptr, &udesc, handle);
	views[uavIndex] = handle;
}

void BufferResource::Release()
{
	resource->Release();
	// add to freeList
	Free();
}

/************************************************************************/
// TextureResource
/************************************************************************/

void TextureResource::Upload(ID3D12Device* d3d12Device, D3D12CommandContext* cmdContext, UploadHeap* uploadHeap, std::vector<D3D12_SUBRESOURCE_DATA>& subresources)
{
	// upload data to resource
	auto cmdList = cmdContext->GetCmdList();
	UpdateSubresources(cmdList, resource, uploadHeap->Get(),
		0, 0, static_cast<UINT>(subresources.size()), subresources.data());
}

void TextureResource::Create(ID3D12Device* d3d12Device, ResourceDescribe* resourceDesc)
{
	auto& textureDesc = resourceDesc->textureDesc;
	if (textureDesc.CpuData && textureDesc.Size) {
		// create texture from dds data
		std::vector<D3D12_SUBRESOURCE_DATA > subresources;
		HRESULT result = LoadDDSTextureFromMemory(d3d12Device, (uint8_t*)textureDesc.CpuData, textureDesc.Size, &resource, subresources, 0, NULL, &isCube);
		// set resource initial state
		state = D3D12_RESOURCE_STATE_COPY_DEST;
		// get resource size
		auto uploadBufferSize = GetRequiredIntermediateSize(resource, 0, static_cast<UINT>(subresources.size()));
		auto copyContext = D3D12CommandContext::Alloc(d3d12Device, D3D12_COMMAND_LIST_TYPE_DIRECT);
		auto uploadHeap = UploadHeap::Alloc(d3d12Device, uploadBufferSize);
		Upload(d3d12Device, copyContext, uploadHeap, subresources);
		// resource barrier
		auto targetState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		// set current state
		SetResourceState(copyContext, targetState);
		// release temp resource
		copyContext->Flush(true);
		copyContext->Release();
		uploadHeap->Release();
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

		D3D12_RESOURCE_STATES createState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		D3D12_CLEAR_VALUE clear = {};
		D3D12_CLEAR_VALUE* pClear = nullptr;  

		clear.Format = resourceDesc.Format;
		if (textureDesc.BindFlag & BIND_DEPTH_STENCIL) {
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			createState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
			// clearvalue
			pClear = &clear;
			clear.DepthStencil.Depth = 1;
			clear.DepthStencil.Stencil = 0;
			if (clear.Format == FORMAT_R32_TYPELESS) {
				clear.Format = (DXGI_FORMAT)FORMAT_D32_FLOAT;
			}
		} else if (textureDesc.BindFlag & BIND_RENDER_TARGET) {
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			createState = D3D12_RESOURCE_STATE_RENDER_TARGET;
			// clearvalue
			pClear = &clear;
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
	}
	// set debug name
	resource->SetName(textureDesc.DebugName);
}

void TextureResource::CreateViews(ID3D12Device* d3d12Device, ResourceDescribe* resourceDesc, D3D12DescriptorHeap** descHeaps)
{
	auto& textureDesc = resourceDesc->textureDesc;
	// rtv
	if (textureDesc.BindFlag & BIND_RENDER_TARGET) {
		D3D12_RENDER_TARGET_VIEW_DESC rtDesc = {};
		rtDesc.Format = (DXGI_FORMAT)textureDesc.Format;
		rtDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtDesc.Texture2D.MipSlice = 0;
		auto constexpr rtvIndex = (int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::RTV;
		auto handle = descHeaps[rtvIndex]->GetCpuHandle(resourceId & 0x00ffffff);
		d3d12Device->CreateRenderTargetView(resource, &rtDesc, handle);
		views[rtvIndex] = handle;
		// rtvFormat
		rtvFormat = rtDesc.Format;
	}
	// dsv
	if (textureDesc.BindFlag & BIND_DEPTH_STENCIL) {
		D3D12_DEPTH_STENCIL_VIEW_DESC dsDesc = {};
		dsDesc.Format = (DXGI_FORMAT)textureDesc.Format;
		if (textureDesc.Format == FORMAT_R32_TYPELESS) {
			dsDesc.Format = (DXGI_FORMAT)FORMAT_D32_FLOAT;
		}
		dsDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsDesc.Texture2D.MipSlice = 0;
		
		auto constexpr dsvIndex = (int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::DSV;
		auto handle = descHeaps[dsvIndex]->GetCpuHandle(resourceId & 0x00ffffff);
		d3d12Device->CreateDepthStencilView(resource, &dsDesc, handle);
		views[dsvIndex] = handle;
		// dsvFormat
		dsvFormat = dsDesc.Format;
	}
	// uav
	if (textureDesc.BindFlag & BIND_UNORDERED_ACCESS) {
		D3D12_UNORDERED_ACCESS_VIEW_DESC udesc = {};
		udesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		udesc.Format = (DXGI_FORMAT)textureDesc.Format;
		udesc.Texture2D.MipSlice = 0;
		udesc.Texture2D.PlaneSlice = 0;
		auto constexpr uavIndex = (int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::UAV;
		auto handle = descHeaps[uavIndex]->GetCpuHandle(resourceId & 0x00ffffff);
		d3d12Device->CreateUnorderedAccessView(resource, nullptr, &udesc, handle);
		views[uavIndex] = handle;
	}
	//  create srv
	if (textureDesc.BindFlag & BIND_SHADER_RESOURCE) {
		D3D12_SHADER_RESOURCE_VIEW_DESC vdesc = {};
		D3D12_RESOURCE_DESC resDesc = resource->GetDesc();
		vdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		vdesc.Format = (DXGI_FORMAT)textureDesc.Format;
		vdesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		if (textureDesc.Format == FORMAT_R32_TYPELESS) {
			vdesc.Format = (DXGI_FORMAT)FORMAT_R32_FLOAT;
		}
		if (isCube) {
			vdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			vdesc.TextureCube.MipLevels = resDesc.MipLevels;
		} else {
			vdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			vdesc.Texture2D.MipLevels = resDesc.MipLevels;
		}
		auto constexpr srvIndex = (int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::SRV;
		auto handle = descHeaps[srvIndex]->GetCpuHandle(resourceId & 0x00ffffff);
		d3d12Device->CreateShaderResourceView(resource, &vdesc, handle);
		views[srvIndex] = handle;
	}
}

void TextureResource::Release()
{
	resource->Release();
	// add to freeList
	Free();
}

/************************************************************************/
// Geometry
/************************************************************************/

void Geometry::Create(ID3D12Device* d3d12Device, ResourceDescribe* resourceDesc)
{
	auto& geometryDesc = resourceDesc->geometryDesc;
	// create vertext buffer
	R_BUFFER_DESC vertexBufferDesc = {
		geometryDesc.VertexBufferSize,
		DEFAULT,
		BIND_VERTEX_BUFFER,
		CPU_ACCESS_WRITE,
		R_MISC(0),
		geometryDesc.VertexStride,
		geometryDesc.VertexBuffer,
		false,
		geometryDesc.DebugName,
	};
	vertexBuffer = BufferResource::CreateResource(d3d12Device, (ResourceDescribe*)&vertexBufferDesc);
	vertexBuffer->resourceId |= (unsigned int)D3D12Resource::RESOURCE_TYPES::BUFFER << 24;
	// create index buffer
	R_BUFFER_DESC indexBufferDesc = {
		geometryDesc.NumIndices * sizeof(WORD),
		DEFAULT,
		BIND_INDEX_BUFFER,
		CPU_ACCESS_WRITE,
		R_MISC(0),
		sizeof(WORD),
		geometryDesc.IndexBuffer,
		false,
		geometryDesc.DebugName,
	};
	indexBuffer = BufferResource::CreateResource(d3d12Device, (ResourceDescribe*)&indexBufferDesc);
	indexBuffer->resourceId |= (unsigned int)D3D12Resource::RESOURCE_TYPES::BUFFER << 24;
	// set property
	vertexStride = geometryDesc.VertexStride;
	// vertex size
	vertexBufferSize = geometryDesc.VertexBufferSize;
	// index num
	numIndices = geometryDesc.NumIndices;
	// toplogy format
	primitiveToplogy = geometryDesc.Top;
}

void Geometry::Release()
{
	vertexBuffer->Release();
	indexBuffer->Release();
	// free rtGeometries
	while (transientRtGeometries.Size()) {
		auto rtGeometry = transientRtGeometries.PopBack();
		rtGeometry->Release();
	}
	if (staticRtGeometry) {
		staticRtGeometry->Release();
	}
	// add to freeList
	Free();
}


int Geometry::CreateRtGeometry(ID3D12Device* d3d12Device, bool isTransient)
{
	RaytracingGeomtry* rtGeometry = nullptr;
	if (isTransient && transientRtGeometries.Size()) {
		rtGeometry = transientRtGeometries.PopBack();
	} else if (isTransient || !staticRtGeometry) {
		// not found, create a new one
		R_RT_GEOMETRY_DESC desc{};
		desc.DebugName = L"blas";
		desc.geometryId = resourceId;
		desc.transient = isTransient;
		rtGeometry = RaytracingGeomtry::CreateResource(d3d12Device, (ResourceDescribe*)&desc);
		rtGeometry->resourceId |= (unsigned int)D3D12Resource::RESOURCE_TYPES::BLAS << 24;
	} else {
		rtGeometry = staticRtGeometry;
	}
	if (isTransient) {
		rtGeometry->SetTransient();
	} else {
		// static geometry
		staticRtGeometry = rtGeometry;
	}
	return rtGeometry->resourceId;
}


/************************************************************************/
// raytracing geometry
/************************************************************************/

Vector<RaytracingGeomtry*> RaytracingGeomtry::inflightRtGeometries;

void RaytracingGeomtry::RetireAllTransientGeometry()
{
	while (inflightRtGeometries.Size()) {
		auto rtGeomtry = inflightRtGeometries.PopBack();
		rtGeomtry->Retire();
	}
}

void RaytracingGeomtry::Create(ID3D12Device* d3d12Device, ResourceDescribe* resourceDesc)
{
	auto rtDesc = (R_RT_GEOMETRY_DESC*)resourceDesc;
	// set parent
	geometry = Geometry::Get(rtDesc->geometryId);
	if (rtDesc->transient) {
		// create transient buffer
		R_BUFFER_DESC bufferDesc{};
		bufferDesc.BindFlags = R_BIND_FLAG::BIND_UNORDERED_ACCESS;
		bufferDesc.Size = geometry->vertexBufferSize;
		bufferDesc.StructureByteStride = geometry->vertexStride;
		bufferDesc.DebugName = L"transient-geometry-buffer";
		transientBuffer = BufferResource::CreateResource(d3d12Device, (ResourceDescribe*)&bufferDesc);
		transientBuffer->CreateViews(d3d12Device, (ResourceDescribe*)&bufferDesc, D3D12RenderInterface::Get()->GetBufferHeaps());
		// get srv 
		auto constexpr srvIndex = (int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::SRV;
		views[srvIndex] = transientBuffer->GetSrv();
		// get uav
		auto constexpr  uavIndex = (int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::UAV;
		views[uavIndex] = transientBuffer->GetUav();
	} 
	// create blas buffer and scratch buffer
	{
		// input
		geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		geometryDesc.Triangles.IndexBuffer = geometry->indexBuffer->GetResource()->GetGPUVirtualAddress();
		geometryDesc.Triangles.IndexCount = geometry->numIndices;
		geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
		geometryDesc.Triangles.Transform3x4 = 0;
		geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		geometryDesc.Triangles.VertexCount = geometry->vertexBufferSize / geometry->vertexStride;
		if (rtDesc->transient) {
			geometryDesc.Triangles.VertexBuffer.StartAddress = transientBuffer->GetResource()->GetGPUVirtualAddress();
		} else {
			geometryDesc.Triangles.VertexBuffer.StartAddress = geometry->vertexBuffer->GetResource()->GetGPUVirtualAddress();
		}
		geometryDesc.Triangles.VertexBuffer.StrideInBytes = geometry->vertexStride;
		geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

		bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		auto flags = rtDesc->transient ?
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE :
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION;
		bottomLevelInputs.Flags = flags;
		bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		bottomLevelInputs.pGeometryDescs = &geometryDesc;
		bottomLevelInputs.NumDescs = 1;

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};

		ID3D12Device5* rtxDevice = nullptr;
		d3d12Device->QueryInterface(IID_PPV_ARGS(&rtxDevice));
		// get resource size needed
		rtxDevice->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);
		rtxDevice->Release();
		// create scratch buffer
		scratchBuffer = CreateCommittedResource(d3d12Device,
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(bottomLevelPrebuildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
				nullptr);
		// create blas buffer
		asBuffer = CreateCommittedResource(d3d12Device,
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
			D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
			nullptr);
		scratchBuffer->SetName(L"scratch-buffer");
		asBuffer->SetName(L"blas");
	}
}

void RaytracingGeomtry::SetTransient()
{
	// need re-initialize
	initialized = false;
	// set transient
	isTransient = true;
	// add this to inflight vector
	auto rtGeometry = this;
	inflightRtGeometries.PushBack(rtGeometry);
}

void RaytracingGeomtry::SetResourceState(D3D12CommandContext* cmdContext, D3D12_RESOURCE_STATES targetState)
{
	if (transientBuffer) {
		transientBuffer->SetResourceState(cmdContext, targetState);
	}
}

void RaytracingGeomtry::PreBuild(D3D12CommandContext* cmdContext)
{
	if (initialized || !isTransient) {
		// already build
		return;
	}
	// add barriers
	if (transientBuffer->GetResourceState() != D3D12_RESOURCE_STATE_COPY_DEST) {
		// make sure the transientBuffer is initialized
		constexpr auto state = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		transientBuffer->SetResourceState(cmdContext, state);
	}
}

void RaytracingGeomtry::Build(D3D12CommandContext* cmdContext)
{
	if (initialized) {
		return;
	}
	if (isTransient && transientBuffer->GetResourceState() == D3D12_RESOURCE_STATE_COPY_DEST) {
		// transientBuffer is no initialized
		return;
	}
	auto rtCmdList = cmdContext->GetRtCmdList();
	// get build desc
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
	buildDesc.DestAccelerationStructureData = asBuffer->GetGPUVirtualAddress();
	buildDesc.ScratchAccelerationStructureData = scratchBuffer->GetGPUVirtualAddress();
	buildDesc.SourceAccelerationStructureData = 0;
	buildDesc.Inputs = bottomLevelInputs;
	rtCmdList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);
	// add barriers
	auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(asBuffer);
	cmdContext->AddBarrier(barrier);
	initialized = true;
}

void RaytracingGeomtry::PostBuild(D3D12CommandContext* cmdContext)
{
}


void RaytracingGeomtry::Retire()
{
	if (isTransient) {
		auto rtGeometry = this;
		geometry->transientRtGeometries.PushBack(rtGeometry);
	}
}

void RaytracingGeomtry::Release()
{
	if (transientBuffer) {
		transientBuffer->Release();
	}
	if (asBuffer) {
		asBuffer->Release();
	}
	if (scratchBuffer) {
		scratchBuffer->Release();
	}
	Free();
}


/************************************************************************/
// rt Scene
/************************************************************************/

RaytracingScene* RaytracingScene::AllocTransient(ID3D12Device* d3d12Device)
{
	auto rtScene = allocTransient(
		[&](RaytracingScene* newRtScene) {
			newRtScene->create(d3d12Device);
		},
		[&](RaytracingScene* newRtScene) {
			return true;
		}
		);
	return rtScene;
}


void RaytracingScene::AddInstance(RaytracingGeomtry* rtGeometry, Matrix4x4& transform)
{
	// instanceDesc.PushBack(instance);
	auto blas = rtGeometry->GetBottomLevel();
	D3D12_RAYTRACING_INSTANCE_DESC instance{};
	instance.AccelerationStructure = blas->GetGPUVirtualAddress();
	instance.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE;
	instance.InstanceID = 0;
	instance.InstanceMask = 1;
	instance.InstanceContributionToHitGroupIndex = instanceDesc.Size();
	// set transform 
	Matrix4x4 trans;
	transform.Tranpose(transform, &trans);
	memcpy(instance.Transform, &trans, sizeof(float) * 12);
	// push instance
	instanceDesc.PushBack(instance);
	bottomLevelGeometries.PushBack(rtGeometry);
}


void RaytracingScene::create(ID3D12Device* d3d12Device)
{
	// create top level as
	topLevelAs = CreateCommittedResource(d3d12Device,
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(default_top_level_as_size, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
		nullptr);
	topLevelAs->SetName(L"tlas");
	topLevelSize = default_top_level_as_size;
	// create blas buffer
	topLevelScratch = CreateCommittedResource(d3d12Device,
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(default_top_level_as_size, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		nullptr);
	topLevelScratch->SetName(L"toplevel-scratch");
	scratchSize = default_top_level_as_size;
	// create instance buffer
	instanceBuffer = CreateCommittedResource(d3d12Device,
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(default_top_level_as_size, D3D12_RESOURCE_FLAG_NONE),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr);
	instanceBuffer->SetName(L"rt-instance");
	instanceSize = default_top_level_as_size;
	instanceBuffer->Map(0, nullptr, &instancePtr);
	// create heaps
	descHeap = D3D12DescriptorHeap::Alloc(d3d12Device, 16, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.RaytracingAccelerationStructure.Location = topLevelAs->GetGPUVirtualAddress();
	desc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	d3d12Device->CreateShaderResourceView(nullptr, &desc, descHeap->GetCpuHandle(0));
	// create sbt
	sbt.Create(d3d12Device);
	// set device
	this->d3d12Device = d3d12Device;
	d3d12Device->QueryInterface(IID_PPV_ARGS(&rtxDevice));
}

void RaytracingScene::resetTransient()
{
	instanceDesc.Reset();
	bottomLevelGeometries.Reset();
}

void RaytracingScene::Build(D3D12CommandContext* cmdContext)
{
	// prebuild (set barriers)
	for (auto iter = bottomLevelGeometries.Begin(); iter != bottomLevelGeometries.End(); iter++) {
		auto blas = *iter;
		blas->PreBuild(cmdContext);
	}
	cmdContext->ApplyBarriers();
	// build
	for (auto iter = bottomLevelGeometries.Begin(); iter != bottomLevelGeometries.End(); iter++) {
		auto blas = *iter;
		blas->Build(cmdContext);
	}
	// build top level
	// 1. copy instance desc to upload buffer
	if (instanceDesc.Size()) {

		auto instanceSize = instanceDesc.Size() * sizeof(D3D12_RAYTRACING_INSTANCE_DESC);
		if (instanceSize > this->instanceSize) {
			// resize instance buffer
			instanceBuffer->Unmap(0, nullptr);
			instanceBuffer->Release();
			instanceBuffer = CreateCommittedResource(d3d12Device,
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(instanceSize, D3D12_RESOURCE_FLAG_NONE),
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr);
			this->instanceSize = instanceSize;
			instanceBuffer->SetName(L"rt-instance");
			instanceBuffer->Map(0, nullptr, &instancePtr);
		}
		memcpy(instancePtr, instanceDesc.GetData(), instanceSize);
	}
	cmdContext->ApplyBarriers();
	// 2. get toplevel size
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {};
	topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	topLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	topLevelInputs.NumDescs = instanceDesc.Size();
	topLevelInputs.pGeometryDescs = nullptr;
	topLevelInputs.ppGeometryDescs = nullptr;
	topLevelInputs.InstanceDescs = instanceBuffer->GetGPUVirtualAddress();
	topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};

	rtxDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);

	// 3. reallocate top level data size
	if (topLevelPrebuildInfo.ScratchDataSizeInBytes > scratchSize) {
		topLevelScratch->Release();
		topLevelScratch = CreateCommittedResource(d3d12Device,
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(topLevelPrebuildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr);
		topLevelScratch->SetName(L"toplevelscratch");
		scratchSize = topLevelPrebuildInfo.ScratchDataSizeInBytes;
	}
	if (topLevelPrebuildInfo.ResultDataMaxSizeInBytes > topLevelSize) {
		topLevelAs->Release();
		topLevelAs = CreateCommittedResource(d3d12Device,
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(topLevelPrebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS),
			D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
			nullptr);
		topLevelAs->SetName(L"tlas");
		topLevelSize = topLevelPrebuildInfo.ResultDataMaxSizeInBytes;
		// create srv
		D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
		desc.RaytracingAccelerationStructure.Location = topLevelAs->GetGPUVirtualAddress();
		desc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		d3d12Device->CreateShaderResourceView(nullptr, &desc, descHeap->GetCpuHandle(0));
	}
	// 4. build top level as
	auto cmdList = cmdContext->GetRtCmdList();

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
	buildDesc.DestAccelerationStructureData = topLevelAs->GetGPUVirtualAddress();
	buildDesc.ScratchAccelerationStructureData = topLevelScratch->GetGPUVirtualAddress();
	buildDesc.SourceAccelerationStructureData = 0;
	buildDesc.Inputs = topLevelInputs;
	cmdList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

	// 6. uav barriar
	CD3DX12_RESOURCE_BARRIER UAVBarriers[2] = { CD3DX12_RESOURCE_BARRIER::UAV(topLevelAs),  CD3DX12_RESOURCE_BARRIER::UAV(topLevelScratch) };
	cmdList->ResourceBarrier(2, UAVBarriers);
}

void RaytracingScene::TraceRay(D3D12CommandContext* cmdContext, int shaderIndex, unsigned int width, unsigned int height)
{
	// TODO: 
	auto rtCmdList = cmdContext->GetRtCmdList();
	// flush sbt
	// refresh the rtpso
	stateObject.Refresh(rtxDevice);
	rtCmdList->SetPipelineState1(stateObject.Get());
}


/************************************************************************/
// UploadHeap
/************************************************************************/

UploadHeap* UploadHeap::Alloc(ID3D12Device* d3d12Device, UINT64 size)
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

void UploadHeap::create(ID3D12Device* d3d12Device, UINT64 size)
{
	D3D12_HEAP_TYPE heapType{};
	resource = CreateCommittedResource(d3d12Device,
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(size, D3D12_RESOURCE_FLAG_NONE),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr);
	resource->SetName(L"upload-heap");
	this->size = size;
}

void UploadHeap::Release()
{
	resetTransient();
	if (cpuBaseAddress) {
		resource->Unmap(0, nullptr);
		cpuBaseAddress = nullptr;
	}
	resource->Release();
	delete this;
}

bool UploadHeap::SubAlloc(unsigned int allocSize)
{
	// make the resource mapped
	if (!cpuBaseAddress) {
		CD3DX12_RANGE readRange(0, 0);
		resource->Map(0, &readRange, &cpuBaseAddress);
	}
	int alignedSize = (allocSize + const_buffer_align - 1) & ~(const_buffer_align - 1);

	if ((UINT64)currentRear + alignedSize > size) {
		// not enough space
		return false;
	}
	// 
	currentOffset = currentRear;
	currentRear += alignedSize;
	return true;
}

void* UploadHeap::GetCurrentCpuVirtualAddress() {
	return (void*)((UINT64)cpuBaseAddress + (UINT64)currentOffset); 
}

D3D12_GPU_VIRTUAL_ADDRESS UploadHeap::GetCurrentGpuVirtualAddress() { 
	return resource->GetGPUVirtualAddress() + (UINT64)currentOffset; 
}


/************************************************************************/
// Ring Constant Buffer
/************************************************************************/


/*
*	arguments:
*		size:             buffer size to allocate
*		gpuAddress(out):  pointer to gpu address
*	return:
*		cpu address
*/
void* RingConstantBuffer::AllocTransientConstantBuffer(unsigned int size, D3D12_GPU_VIRTUAL_ADDRESS* gpuAddress)
{
	// suballoc constant buffer space
	if (!currentUploadHeap || !currentUploadHeap->SubAlloc(size)) {
		// alloc a new transient const buffer
		currentUploadHeap = UploadHeap::AllocTransient(d3d12Device, max_upload_heap_size);
		if (!currentUploadHeap->SubAlloc(size)) {
			// no space left, error
			*gpuAddress = 0;
			return nullptr;
		}
	}
	*gpuAddress = currentUploadHeap->GetCurrentGpuVirtualAddress();
	return currentUploadHeap->GetCurrentCpuVirtualAddress();
}

RingConstantBuffer* RingConstantBuffer::Alloc(ID3D12Device* d3d12Device) {
	auto ring = new RingConstantBuffer();
	ring->d3d12Device = d3d12Device;
	return ring;
}

void RingConstantBuffer::Release()
{
	delete this;
}

void RingConstantBuffer::Reset()
{
	currentUploadHeap = nullptr;
}


/************************************************************************/
// BackBuffer
/************************************************************************/

void D3D12BackBuffer::Create(ID3D12Device* d3d12Device, IDXGIFactory4* pFactory, HWND hWnd, int width, int height, D3D12DescriptorHeap* rtvHeap)
{

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = backbuffer_count;
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;
	// full screen
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenChain = {};
	fullScreenChain.RefreshRate.Denominator = 1;
	fullScreenChain.RefreshRate.Numerator = 60;
	fullScreenChain.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	fullScreenChain.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	fullScreenChain.Windowed = 1;

	// create swapchain
	IDXGISwapChain1* tempSwapChain;
	if (FAILED(pFactory->CreateSwapChainForHwnd(
		D3D12CommandQueue::GetQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)->Get(),		// Swap chain needs the queue so that it can force a flush on it.
		hWnd,
		&swapChainDesc,
		&fullScreenChain,
		NULL,
		&tempSwapChain
	))) {
		printf("Failed to create swapchain\n");
		return;
	}

	// does not support fullscreen transitions.
	pFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);
	tempSwapChain->QueryInterface(IID_PPV_ARGS(&swapChain));
	frameIndex = swapChain->GetCurrentBackBufferIndex();

	for (auto n = 0; n < backbuffer_count; n++) {
		// alloc texture id 0. for reservedId. id zero is backbuffer
		auto reservedId = TextureResource::Alloc();
		// get backbuffer resource
		swapChain->GetBuffer(n, IID_PPV_ARGS(&backBuffers[n].resource));
		auto& textureResource = backBuffers[n];
		textureResource.rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		textureResource.state = D3D12_RESOURCE_STATE_PRESENT;
		// create views
		D3D12_RENDER_TARGET_VIEW_DESC vdesc = {};
		vdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		vdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		vdesc.Texture2D.MipSlice = 0;
		vdesc.Texture2D.PlaneSlice = 0;
		auto handle = rtvHeap->GetCpuHandle(n);
		d3d12Device->CreateRenderTargetView(textureResource.resource, &vdesc, handle);
		textureResource.views[(unsigned int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::RTV] = handle;
	}
	tempSwapChain->Release();
	// set size
	this->width = width;
	this->height = height;
	this->hWnd = hWnd;
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12BackBuffer::GetRtv()
{
	auto& textureResource = backBuffers[frameIndex];
	return textureResource.GetRtv();
}

ID3D12Resource* D3D12BackBuffer::GetResource()
{
	auto& textureResource = backBuffers[frameIndex];
	return textureResource.GetResource();
}

void D3D12BackBuffer::SetResourceState(D3D12CommandContext* cmdContext, D3D12_RESOURCE_STATES targetState)
{
	auto& textureResource = backBuffers[frameIndex];
	textureResource.SetResourceState(cmdContext, targetState);
}

UINT64 D3D12BackBuffer::Present(D3D12CommandContext* cmdContext)
{
	// Indicate that the back buffer will be used as present.
	D3D12_RESOURCE_BARRIER barrier = {};
	// Indicate that the back buffer will now be used to present.
	auto& textureResource = backBuffers[frameIndex];
	textureResource.SetResourceState(cmdContext, D3D12_RESOURCE_STATE_PRESENT);
	// test code. test async compute function
	UINT64 fenceValue = cmdContext->Flush(0);
	swapChain->Present(1, 0);
	prevFrameFence[frameIndex] = fenceValue;
	// wait for prev frame to finish
	WaitForNextFrame();
	return fenceValue;
}

void D3D12BackBuffer::WaitForNextFrame() 
{
	// get new frame index
	frameIndex = swapChain->GetCurrentBackBufferIndex();
	UINT64 fenceToWait = prevFrameFence[frameIndex];
	// wait for prev frame
	auto cmdQueue = D3D12CommandQueue::GetQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	cmdQueue->CpuWait(fenceToWait);
}

/************************************************************************/
// BackBuffer
/************************************************************************/


ShaderRecord* ShaderBindingTable::AllocShaderRecord(int materialId)
{
	// we ignore materialId for now
	ShaderRecord Record{};
	int index = hitGroups.PushBack(Record);
	return &hitGroups[index];
}

void ShaderBindingTable::Reset()
{
	hitGroups.Reset();
}


void ShaderBindingTable::Create(ID3D12Device* d3d12Device)
{
	// create sbt table
	sbt = CreateCommittedResource(d3d12Device,
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(default_sbt_size, D3D12_RESOURCE_FLAG_NONE),
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		nullptr);
	sbt->SetName(L"sbt");
	sbtSize = default_sbt_size;
	// create sbt cpu buffer
	sbtCpu = CreateCommittedResource(d3d12Device,
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(default_top_level_as_size, D3D12_RESOURCE_FLAG_NONE),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr);
	sbtCpu->SetName(L"sbt-cpu");
	sbtCpu->Map(0, nullptr, &sbtPtr);
}