#include "D3D12Renderer.h"
#include "Tasks/ThreadLocal.h"

using namespace D3D12Renderer;

/************************************************************************/
// helpers
/************************************************************************/

R_PRIMITIVE_TOPOLOGY_TYPE getPtimitiveTopologyType(R_PRIMITIVE_TOPOLOGY topology) {
	if (topology == R_PRIMITIVE_TOPOLOGY_UNDEFINED) {
		return R_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
	}
	else if (topology == R_PRIMITIVE_TOPOLOGY_POINTLIST) {
		return R_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	}
	else if (topology == R_PRIMITIVE_TOPOLOGY_LINELIST || topology == R_PRIMITIVE_TOPOLOGY_LINESTRIP ||
		topology == R_PRIMITIVE_TOPOLOGY_LINELIST_ADJ || topology == R_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ) {
		return R_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	}
	else if (topology == R_PRIMITIVE_TOPOLOGY_TRIANGLELIST || topology == R_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP ||
		topology == R_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ || topology == R_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ) {
		return R_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	}
	else if (topology == R_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST || topology == R_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST ||
		topology == R_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST || topology == R_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST) {
		return R_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	}
	return R_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
}


/************************************************************************/
// Memory
/************************************************************************/

Memory* Memory::Alloc(unsigned int size)
{
	auto data = new char[size];
	return new Memory{ data, size };
}


void Memory::Free()
{
	if (m_data) {
		delete m_data;
		m_data = nullptr;
		m_size = 0;
	}
	delete this;
}

Memory::~Memory()
{
	if (m_data) {
		delete m_data;
	}
}

void Memory::Resize(unsigned int size)
{
	if (size <= m_size) {
		// nothing todo
		return;
	}
	// exp 
	auto newSize = 0;
	if (m_size && size > m_size) {
		newSize = size * 2;
		auto data = new char[newSize];
		memcpy(data, m_data, m_size);
		delete m_data;
		m_data = data;
	} else {
		newSize = size;
		m_data = new char[newSize];
	}
	m_size = newSize;
}


/************************************************************************/
// ConstantBufferCache
/************************************************************************/

void ConstantCache::Update(int slot, unsigned int offset, void* buffer, unsigned int size)
{
	auto& constant = constantBuffer[slot];
	constant.Resize(offset + size);
	memcpy((unsigned char*)constant.m_data + offset, buffer, size);
	// mark slot dirty
	dirty[slot] = true;
}


void ConstantCache::Upload(int slot, void* cpuDst, unsigned int size)
{
	if (dirty[slot]) {
		auto& constant = constantBuffer[slot];
		//printf("copy %lx %lx copy size:%ld, total size: %ld\n", cpuDst, constant.m_data, size, constant.m_size);
		memcpy(cpuDst, constant.m_data, constant.m_size);
		// clear dirty flag
		dirty[slot] = false;
	}
}


/************************************************************************/
// CommandContext
/************************************************************************/

void D3D12CommandContext::create(ID3D12Device* d3d12Device, D3D12_COMMAND_LIST_TYPE cmdType, D3D12DescriptorHeap* samplerHeap, D3D12DescriptorHeap* nullHeap)
{
	d3d12Device->CreateCommandAllocator(cmdType, IID_PPV_ARGS(&cmdAllocator));
	if (cmdAllocator) {
		d3d12Device->CreateCommandList(0, cmdType, cmdAllocator, nullptr, IID_PPV_ARGS(&cmdList));
		cmdList->QueryInterface(IID_PPV_ARGS(&rtCommandList));
	}
	this->cmdType = cmdType;
	this->samplerHeap = samplerHeap;
	this->nullHeap = nullHeap;
	this->d3d12Device = d3d12Device;
}

D3D12CommandContext* D3D12CommandContext::AllocTransient(ID3D12Device* d3d12Device, D3D12_COMMAND_LIST_TYPE cmdType, D3D12DescriptorHeap** descHeaps)
{
	auto cmdContext = allocTransient(
		[&](D3D12CommandContext* newContext) {
			newContext->create(d3d12Device, cmdType, descHeaps[(int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::SAMPLER], descHeaps[(int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::UNBOUND]);
		},
		[&](D3D12CommandContext* retiredContext) {
			return retiredContext->cmdType == cmdType;
		}
	);
	cmdContext->initialize();
	return cmdContext;
}

D3D12CommandContext* D3D12CommandContext::Alloc(ID3D12Device* d3d12Device, D3D12_COMMAND_LIST_TYPE cmdType)
{
	auto cmdContext = new D3D12CommandContext();
	// temp context don't use samplers
	cmdContext->create(d3d12Device, cmdType, nullptr, nullptr);
	return cmdContext;
}

void D3D12CommandContext::bindDescriptorHeap(D3D12DescriptorHeap* heap)
{
	ID3D12DescriptorHeap* heaps[2] = { heap->Get(), samplerHeap->Get() };
	cmdList->SetDescriptorHeaps(2, heaps);
	currentRootSignature->SetSamplerTable(cmdList, heaps[1]->GetGPUDescriptorHandleForHeapStart());
}

void D3D12CommandContext::initialize()
{
	// ensure rootsignatures
	if (!graphicsRootSignature && cmdType == D3D12_COMMAND_LIST_TYPE_DIRECT) {
		graphicsRootSignature = D3D12RootSignature::AllocTransient(d3d12Device, false, false, nullHeap);
		cmdList->SetGraphicsRootSignature(graphicsRootSignature->Get());
	}
	if (!computeRootSignature) {
		computeRootSignature = D3D12RootSignature::AllocTransient(d3d12Device, false, true, nullHeap);
		cmdList->SetComputeRootSignature(computeRootSignature->Get());
	}
	// defualt to graphics context
	if (cmdType == D3D12_COMMAND_LIST_TYPE_DIRECT) {
		SetGraphicsMode();
	} else if (cmdType == D3D12_COMMAND_LIST_TYPE_COMPUTE) {
		SetComputeMode();
		isAsyncCompute = true;
	}
	// ensure descriptor heaps
	if (!descriptorHeap) {
		descriptorHeap = D3D12DescriptorHeap::AllocTransient(d3d12Device);
		bindDescriptorHeap(descriptorHeap);
	}
	// ensure constant buffers
	if (!ringConstantBuffer) {
		ringConstantBuffer = RingConstantBuffer::Alloc(d3d12Device);
	}
	// pipeline state default to dirty
	pipelineStateCache.Dirty = true;
}

void D3D12CommandContext::SetGraphicsMode()
{
	if (mode == Mode::GRAPHICS) {
		// nothing to change
		return;
	}
	if (cmdType == D3D12_COMMAND_LIST_TYPE_COMPUTE) {
		// can't set to graphics mode with compute queue
		return;
	}
	isCompute = false;
	graphicsRootSignature->SetStale();
	cmdList->SetGraphicsRootSignature(graphicsRootSignature->Get());
	cmdList->SetComputeRootSignature(nullptr);
	currentRootSignature = graphicsRootSignature;
	// clear compute shader in pso
	pipelineStateCache.CS = -1;
	// bind heaps
	if (descriptorHeap) {
		bindDescriptorHeap(descriptorHeap);
	}
	mode = Mode::GRAPHICS;
}

void D3D12CommandContext::SetComputeMode()
{
	if (mode == Mode::COMPUTE) {
		// nothing to change
		return;
	}
	if (cmdType == D3D12_COMMAND_LIST_TYPE_DIRECT) {
		cmdList->SetGraphicsRootSignature(nullptr);
	}
	computeRootSignature->SetStale();
	cmdList->SetComputeRootSignature(computeRootSignature->Get());
	currentRootSignature = computeRootSignature;
	// cleae ps and vs in pso
	pipelineStateCache.VS = pipelineStateCache.PS = -1;
	// bind heaps
	if (descriptorHeap) {
		bindDescriptorHeap(descriptorHeap);
	}
	isCompute = true;
	mode = Mode::COMPUTE;
}

void D3D12CommandContext::SetRaytracingMode()
{
	if (mode == Mode::RAYTRACING) {
		// nothing to change
		return;
	}
	// set to compute mode 
	SetComputeMode();
	// bind the raytracing heap
	auto rtScene = D3D12RenderInterface::Get()->GetRaytracingScene();
	descriptorHeap = rtScene->GetDescriptorHeap();
	bindDescriptorHeap(descriptorHeap);
	currentRootSignature->SetStale();
	mode = Mode::RAYTRACING;
}

void D3D12CommandContext::AddRaytracingInstance(R_RAYTRACING_INSTANCE* instance)
{
	auto resourceType = (instance->rtGeometry & 0xff000000) >> 24;
	RaytracingGeomtry* rtGeometry = nullptr;
	if (resourceType == static_cast<int>(D3D12Resource::RESOURCE_TYPES::GEOMETRY)) {
		auto geometry = Geometry::Get(instance->rtGeometry);
		geometry->CreateRtGeometry(d3d12Device, false);
		rtGeometry = geometry->staticRtGeometry;
	} else {
		rtGeometry = RaytracingGeomtry::Get(instance->rtGeometry);
	}
	auto rtScene = D3D12RenderInterface::Get()->GetRaytracingScene();
	// get vertex and index buffer
	D3D12_GPU_VIRTUAL_ADDRESS vertexAddr = rtGeometry->geometry->vertexBuffer->GetResource()->GetGPUVirtualAddress();
	D3D12_GPU_VIRTUAL_ADDRESS indexAddr = rtGeometry->geometry->indexBuffer->GetResource()->GetGPUVirtualAddress();
	// add shader recrod for each ray types
	for (auto ray = 0; ray < instance->NumShaders; ray++) {
		// R_SHADER_RESOURCE_BINDINGS
		R_RAYTRACING_SHADER_BINDINGS& bindings = instance->ShaderBindings[ray];
		// get a shader record
		auto record = rtScene->AllocShaderRecord(instance->MaterialId);
		// fill the hitgroup identifier
		auto shader = RaytracingShader::Get(bindings.ShaderId);
		record->identifier = shader->hitGroup;
		// get raytracing's descriptor heap for local srv uav
		auto descHeap = rtScene->GetDescriptorHeap();
		// get raytracing's local root rs
		auto localRootSignature = rtScene->GetLocalRootSignature();
		// set bindings
		for (auto i = 0; i < bindings.NumBindings; i++) {
			auto binding = bindings.Bindings[i];

			int ResourceIndex = 0;
			int HandleIndex = 0;
			D3D12_CPU_DESCRIPTOR_HANDLE handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT());

			auto resource = D3D12RenderInterface::Get()->GetResource(binding.ResourceId);

			switch (binding.BindingType) {
			case R_SRV_TEXTURE:
			case R_SRV_BUFFER:
				localRootSignature->SetSRV(binding.Slot, resource->GetSrv());
				break;
			case R_UAV_TEXTURE:
			case R_UAV_BUFFER:
				localRootSignature->SetUAV(binding.Slot, resource->GetUav());
				break;
			}
		}
		// set vertex and index buffer rootparams
		record->rootParams[0] = vertexAddr;
		record->rootParams[1] = indexAddr;
		// set vertext stride as local root constant
		*(unsigned int*)&record->rootParams[2] = rtGeometry->geometry->vertexStride;
		// flush localrootsig to sbt and rtScene's local descriptor heap
		localRootSignature->FlushShaderBinginds(cmdList, descHeap, record);
	}
	// add rt instance
	rtScene->AddInstance(rtGeometry, instance->Transform, instance->NumShaders);
}

void D3D12CommandContext::BuildAccelerationStructure()
{
	auto rtScene = D3D12RenderInterface::Get()->GetRaytracingScene();
	rtScene->Build(this);
}

void D3D12CommandContext::resetTransient() {
	// reset cmdList and allocater
	cmdList->Close();
	cmdAllocator->Reset();
	cmdList->Reset(cmdAllocator, nullptr);
	// reset constant buffer allocater
	ringConstantBuffer->Reset();
	// reset rootsignarue
	graphicsRootSignature = nullptr;
	computeRootSignature = nullptr;
	// reset descriptorheap
	descriptorHeap = nullptr;
	// reset barriers
	barriers.Reset();
}

void D3D12CommandContext::Release()
{
	// free all resources
	/*cmdList->Close();
	cmdList->Reset(cmdAllocator, nullptr);
	cmdAllocator->Reset();*/
	if (rtCommandList) {
		rtCommandList->Release();
	}
	cmdList->Close();
	cmdList->Reset(cmdAllocator, nullptr);
	cmdList->Release();
	cmdAllocator->Reset();
	cmdAllocator->Release();
	if (ringConstantBuffer) {
		ringConstantBuffer->Release();
	}
	delete this;
}

void* D3D12CommandContext::allocTransientConstantBuffer(unsigned int size, D3D12_GPU_VIRTUAL_ADDRESS* gpuAddr)
{
	return ringConstantBuffer->AllocTransientConstantBuffer(size, gpuAddr);
}

void D3D12CommandContext::AddBarrier(D3D12_RESOURCE_BARRIER& barrier)
{
	barriers.PushBack(barrier);
}

void D3D12CommandContext::SetConstantBuffer(int slot, unsigned int size)
{
	// alloc transient constant buffer
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddr{};
	auto cpuData = allocTransientConstantBuffer(size, &gpuAddr);
	constantCache.Upload(slot, cpuData, size);
	// set cbv to current rootsignature
	currentRootSignature->SetConstantBuffer(slot, gpuAddr, size);
}

void D3D12CommandContext::SetSRV(int slot, int resourceId)
{
	if (resourceId != -1) {
		auto resource = D3D12RenderInterface::Get()->GetResource(resourceId);
		currentRootSignature->SetSRV(slot, resource->GetSrv());
		resource->SetResourceState(this, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}
}

void D3D12CommandContext::SetRaytracingScene(int slot)
{
	auto rtScene = D3D12RenderInterface::Get()->GetRaytracingScene();
	currentRootSignature->SetSRV(slot, rtScene->GetSrv());
} 


void D3D12CommandContext::SetUAV(int slot, int resourceId)
{
	if (resourceId != -1) {
		auto resource = D3D12RenderInterface::Get()->GetResource(resourceId);
		currentRootSignature->SetUAV(slot, resource->GetUav());
		resource->SetResourceState(this, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	}
} 

void D3D12CommandContext::SetRenderTargets(int* targets, int numTargets, int depth)
{
	currentNumTargets = numTargets;
	for (auto i = 0; i < numTargets; i++) {
		DXGI_FORMAT rtvFormat{};
		D3D12_CPU_DESCRIPTOR_HANDLE handle = {};
		if (targets[i] == 0) {
			// backbuffer
			auto backbuffer = D3D12RenderInterface::Get()->GetBackBuffer();
			handle = backbuffer->GetRtv();
			// barrier
			backbuffer->SetResourceState(this, D3D12_RESOURCE_STATE_RENDER_TARGET);
			// rtvFormat = backbuffer->GetResource()->GetDesc().Format;
			rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		} else {
			// framebuffer
			auto texture = TextureResource::Get(targets[i]);
			handle = texture->GetRtv();
			// barrier
			texture->SetResourceState(this, D3D12_RESOURCE_STATE_RENDER_TARGET);
			rtvFormat = texture->GetResource()->GetDesc().Format;
		}
		// set pso rtv formats
		if (pipelineStateCache.RTVFormat[i] != rtvFormat) {
			pipelineStateCache.RTVFormat[i] = rtvFormat;
			pipelineStateCache.Dirty = true;
		}
		targetsHandles[i] = handle;
	}
	pipelineStateCache.NumRTV = numTargets;
	D3D12_CPU_DESCRIPTOR_HANDLE* pDepthHandle = nullptr;
	if (depth != -1) {
		auto depthBuffer = TextureResource::Get(depth);
		auto handle = depthBuffer->GetDsv();
		depthHandle = handle;
		pDepthHandle = &depthHandle;
		// barrier
		depthBuffer->SetResourceState(this, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		// set pso dsv format
		DXGI_FORMAT dsvFormat = depthBuffer->GetResource()->GetDesc().Format;
		if (dsvFormat == FORMAT_R32_TYPELESS) {
			dsvFormat = (DXGI_FORMAT)FORMAT_D32_FLOAT;
		}
		if (pipelineStateCache.DSVFormat != dsvFormat) {
			pipelineStateCache.DSVFormat = dsvFormat;
			pipelineStateCache.Dirty = true;
		}
	} else {
		pDepthHandle = nullptr;
	}
	cmdList->OMSetRenderTargets(numTargets, targetsHandles, false, pDepthHandle);
}

void D3D12CommandContext::UpdateConstantBuffer(int slot, unsigned int offset, void* buffer, unsigned int size)
{
	constantCache.Update(slot, offset, buffer, size);
}

void D3D12CommandContext::SetRasterizer(int id)
{
	if (pipelineStateCache.Rasterizer != id) {
		pipelineStateCache.Dirty = true;
		pipelineStateCache.Rasterizer = id;
	}
}

void D3D12CommandContext::SetBlendState(int id)
{
	if (pipelineStateCache.Blend != id) {
		pipelineStateCache.Dirty = true;
		pipelineStateCache.Blend = id;
	}
}

void D3D12CommandContext::SetDepthStencilState(int id)
{
	if (pipelineStateCache.Depth != id) {
		pipelineStateCache.Dirty = true;
		pipelineStateCache.Depth = id;
	}
}

void D3D12CommandContext::SetViewPort(int x, int y, int w, int h)
{
	auto& viewPort = D3D12_VIEWPORT{(float)x, (float)y, (float)w, (float)h, 0, 1};
	cmdList->RSSetViewports(1, &viewPort);
	D3D12_RECT rect = {};
	rect.left = 0;
	rect.right = (long)w;
	rect.top = 0;
	rect.bottom = (long)h;
	cmdList->RSSetScissorRects(1, &rect);
}

void D3D12CommandContext::SetVertexShader(int id)
{
	if (pipelineStateCache.VS != id) {
		pipelineStateCache.Dirty = true;
		pipelineStateCache.VS = id;
	}
}

void D3D12CommandContext::SetPixelShader(int id)
{
	if (pipelineStateCache.PS != id) {
		pipelineStateCache.Dirty = true;
		pipelineStateCache.PS = id;
	}
}

void D3D12CommandContext::SetComputeShader(int id)
{
	if (pipelineStateCache.CS != id) {
		pipelineStateCache.Dirty = true;
		pipelineStateCache.CS = id;
	}
}

void D3D12CommandContext::SetInputLayout(int id)
{
	if (pipelineStateCache.InputLayout != id) {
		pipelineStateCache.Dirty = true;
		pipelineStateCache.InputLayout = id;
	}
}

void D3D12CommandContext::Draw(int geometryId)
{
	// get geometry
	auto geometry = Geometry::Get(geometryId);
	if (geometry) { 
		// set topology
		pipelineStateCache.Top = getPtimitiveTopologyType(geometry->primitiveToplogy);
		// flush state
		flushState();
		// set buffers
		D3D12_VERTEX_BUFFER_VIEW vertex{
			  geometry->vertexBuffer->GetResource()->GetGPUVirtualAddress(),
			  geometry->vertexBufferSize,
			  geometry->vertexStride,
		};
		D3D12_INDEX_BUFFER_VIEW index{
			  geometry->indexBuffer->GetResource()->GetGPUVirtualAddress(),
			  geometry->numIndices * sizeof(WORD),
			  DXGI_FORMAT_R16_UINT,
		};
		cmdList->IASetVertexBuffers(0, 1, &vertex);
		cmdList->IASetIndexBuffer(&index);
		cmdList->IASetPrimitiveTopology((D3D_PRIMITIVE_TOPOLOGY)geometry->primitiveToplogy);
		cmdList->DrawIndexedInstanced(geometry->numIndices, 1, 0, 0, 0);
	}
}

void D3D12CommandContext::DrawInstanced(int geometryId, void* instanceBuffer, unsigned int stride, unsigned int numInstances)
{
	// get geometry
	auto geometry = Geometry::Get(geometryId);
	if (geometry) {
		// set topology
		pipelineStateCache.Top = getPtimitiveTopologyType(geometry->primitiveToplogy);
		// flush state
		flushState();
		// make instance data
		D3D12_GPU_VIRTUAL_ADDRESS instanceAddr{};
		auto cpuData = allocTransientConstantBuffer(numInstances * stride, &instanceAddr);
		memcpy(cpuData, instanceBuffer, (size_t)numInstances * stride);
		// set buffers
		D3D12_VERTEX_BUFFER_VIEW vertex[2]{ 
			{
				geometry->vertexBuffer->GetResource()->GetGPUVirtualAddress(),
				geometry->vertexBufferSize,
				geometry->vertexStride,
			},
			{
				instanceAddr,
				numInstances * stride,
				stride,
			}
		};
		D3D12_INDEX_BUFFER_VIEW index{
			  geometry->indexBuffer->GetResource()->GetGPUVirtualAddress(),
			  geometry->numIndices * sizeof(WORD),
			  DXGI_FORMAT_R16_UINT,
		};
		cmdList->IASetVertexBuffers(0, 2, vertex);
		cmdList->IASetIndexBuffer(&index);
		cmdList->IASetPrimitiveTopology((D3D_PRIMITIVE_TOPOLOGY)geometry->primitiveToplogy);
		cmdList->DrawIndexedInstanced(geometry->numIndices, numInstances, 0, 0, 0);
	}
}

void D3D12CommandContext::Quad()
{
	pipelineStateCache.Top = R_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// flush state
	flushState();
	auto geometry = Geometry::Get(0);

	// set buffers
	D3D12_VERTEX_BUFFER_VIEW vertex{
		geometry->vertexBuffer->GetResource()->GetGPUVirtualAddress(),
		geometry->vertexBufferSize,
		geometry->vertexStride,
	};
	D3D12_INDEX_BUFFER_VIEW index{
		geometry->indexBuffer->GetResource()->GetGPUVirtualAddress(),
		geometry->numIndices * sizeof(WORD),
		DXGI_FORMAT_R16_UINT,
	};
	cmdList->IASetVertexBuffers(0, 1, &vertex);
	cmdList->IASetIndexBuffer(&index);
	cmdList->IASetPrimitiveTopology((D3D_PRIMITIVE_TOPOLOGY)geometry->primitiveToplogy);
	cmdList->DrawIndexedInstanced(geometry->numIndices, 1, 0, 0, 0);
}

void D3D12CommandContext::DispatchRays(int shaderId, int width, int height)
{
	// flush state
	flushState();
	// trace ray
	auto rtScene = D3D12RenderInterface::Get()->GetRaytracingScene();
	rtScene->TraceRay(this, shaderId, width, height);
}

void D3D12CommandContext::DispatchCompute(int x, int y, int z)
{
	// flush state
	flushState();
	// dispatch
	cmdList->Dispatch(x, y, z);
}

void D3D12CommandContext::ClearRenderTargets(bool clearTargets, bool clearDepth)
{
	// flush barriers
	if (clearTargets || clearDepth) {
		ApplyBarriers();
	}
	// clear targets
	if (clearTargets) {
		for (int i = 0; i < currentNumTargets; i++) {
			const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
			cmdList->ClearRenderTargetView(targetsHandles[i], clearColor, 0, 0);
		}
	}
	if (clearDepth) {
		cmdList->ClearDepthStencilView(depthHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1, 0, 0, nullptr);
	}
}

void D3D12CommandContext::ApplyBarriers()
{
	if (barriers.Size()) {
		cmdList->ResourceBarrier(barriers.Size(), barriers.GetData());
		// barriers flushed, so invalidate current root signature bindings
		if (currentRootSignature) {
			currentRootSignature->SetStale();
		}
		barriers.Reset();
	}
}


void D3D12CommandContext::flushState()
{	
	// flush barriers
	ApplyBarriers();
	// flush rootsignature
	if (!currentRootSignature->Flush(cmdList, descriptorHeap)) {
		// alloc a new descripter heap
		descriptorHeap = D3D12DescriptorHeap::AllocTransient(d3d12Device);
		bindDescriptorHeap(descriptorHeap);
		// heap changed, set stale
		currentRootSignature->SetStale();
		currentRootSignature->Flush(cmdList, descriptorHeap);
	}
	// flush pso
	if (pipelineStateCache.Dirty) {
		ID3D12PipelineState* pipeline = D3D12PipelineStateCache::GetPipelineState(d3d12Device, currentRootSignature->Get(), pipelineStateCache);
		cmdList->SetPipelineState(pipeline);
		pipelineStateCache.Dirty = 0;
	}
}

// flush
UINT64 D3D12CommandContext::Flush(bool wait)
{
	// flush any pending barriers
	ApplyBarriers();
	// close the list
	cmdList->Close();
	auto cmdQueue = D3D12CommandQueue::GetQueue(cmdType);
	auto fenceValue = cmdQueue->ExecuteCommandList(cmdList);
	// after execute cmdlist, we can reset the cmdlist
	cmdList->Reset(cmdAllocator, nullptr);
	if (wait) {
		cmdQueue->CpuWait(fenceValue);
	}
	return fenceValue;
}

void D3D12CommandContext::Wait(UINT64 syncPoint, bool asyncCompute)
{
	if (asyncCompute == isAsyncCompute) {
		// the queues are in same mode. no need to wait
		return;
	}
	// get queue to wait for
	D3D12CommandQueue* queueToWait = nullptr;
	D3D12CommandQueue* queue = nullptr;
	if (asyncCompute) {
		queueToWait = D3D12CommandQueue::GetQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE);
	} else {
		queueToWait = D3D12CommandQueue::GetQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	}
	auto fence = queueToWait->GetFence();
	// get queue 
	if (isAsyncCompute) {
		queue = D3D12CommandQueue::GetQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE);
	} else {
		queue = D3D12CommandQueue::GetQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	}
	queue->GpuWait(fence, syncPoint);
} 

/************************************************************************/
// DescriptorHeap
/************************************************************************/

void D3D12DescriptorHeap::create(ID3D12Device* d3d12Device, unsigned int size, D3D12_DESCRIPTOR_HEAP_TYPE heapType, D3D12_DESCRIPTOR_HEAP_FLAGS heapFlag)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Flags = heapFlag;
	desc.Type = heapType;
	desc.NodeMask = 0;
	desc.NumDescriptors = heapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER ?  64 : max_descriptor_heap_size;
	d3d12Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap));
	// set memebrs
	this->size = desc.NumDescriptors;
	currentIndex = 0;
	incrementSize = d3d12Device->GetDescriptorHandleIncrementSize(heapType);
	// device
	this->d3d12Device = d3d12Device;
}

D3D12DescriptorHeap* D3D12DescriptorHeap::Alloc(ID3D12Device* d3d12Device, unsigned int size, D3D12_DESCRIPTOR_HEAP_TYPE heapType, D3D12_DESCRIPTOR_HEAP_FLAGS heapFlag)
{
	auto descHeap = new D3D12DescriptorHeap();
	descHeap->create(d3d12Device, size, heapType, heapFlag);
	return descHeap;
}

// alloc transient descriptorheap. must be  D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV heap
D3D12DescriptorHeap* D3D12DescriptorHeap::AllocTransient(ID3D12Device* d3d12Device)
{
	auto descHeap = allocTransient(
		[&](D3D12DescriptorHeap* descHeap) {
			descHeap->create(d3d12Device, max_descriptor_heap_size, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		},
		[&](D3D12DescriptorHeap* heap) {
			return true;
		}
	);
	return descHeap;
}

// release
void D3D12DescriptorHeap::Release()
{
	descriptorHeap->Release();
	delete this;
}

D3D12_GPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::GetGpuHandle(unsigned int index)
{
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetGPUDescriptorHandleForHeapStart(), index, incrementSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::GetCpuHandle(unsigned int index)
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeap->GetCPUDescriptorHandleForHeapStart(), index, incrementSize);
}

D3D12_GPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::StageDescriptors(D3D12_CPU_DESCRIPTOR_HANDLE* handles, int num) {
	// must be a shader visible heap
	auto end = currentIndex + num;
	if (end > size) {
		// not enough space
		return CD3DX12_GPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT());
	}
	// there is enough space
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(GetGpuHandle(0), currentIndex, incrementSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(GetCpuHandle(0), currentIndex, incrementSize);
	currentIndex += num;
	// copy descriptors
	UINT destRangeSize = num;
	UINT srcRangeSize[64]{};
	D3D12_CPU_DESCRIPTOR_HANDLE srcStarts[64]{};
	for (int i = 0; i < num; i++) {
		srcRangeSize[i] = 1;
		srcStarts[i] = handles[i];
	}
	d3d12Device->CopyDescriptors(1, &cpuHandle, &destRangeSize, num, srcStarts, srcRangeSize, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(gpuHandle, 0, incrementSize);
}

bool D3D12DescriptorHeap::HasSpace(int num)
{
	return currentIndex + num <= size;
}

/************************************************************************/
// RootSignature
/************************************************************************/

D3D12RootSignature* D3D12RootSignature::AllocTransient(ID3D12Device* d3d12Device, bool local, bool compute, D3D12DescriptorHeap* nullHeap)
{
	auto rootSignature = allocTransient(
		[&](D3D12RootSignature* rootSignature) {
			rootSignature->create(d3d12Device, local, compute, nullHeap);
		},
		[&](D3D12RootSignature* rootSignature) {
			return rootSignature->local == local && rootSignature->isCompute == compute;
		}
		);
	return rootSignature;
}

D3D12RootSignature* D3D12RootSignature::Alloc(ID3D12Device* d3d12Device, bool local, bool compute, D3D12DescriptorHeap* nullHeap)
{
	auto rootSignature = new D3D12RootSignature();
	rootSignature->create(d3d12Device, local, compute, nullHeap);
	return rootSignature;
}

void D3D12RootSignature::Release()
{
	rootSignature->Release();
}

void D3D12RootSignature::initRootSignature(ID3D12Device* d3d12Device, bool local, D3D12_ROOT_PARAMETER1* rootParameters, int numRootParameters) {

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC RootSig(numRootParameters, rootParameters, 0, 0, local ? D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE : D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	ID3DBlob* pSerializedRootSig;
	ID3DBlob* pError;
	HRESULT result = D3D12SerializeVersionedRootSignature(&RootSig, &pSerializedRootSig, &pError);
	result = d3d12Device->CreateRootSignature(0, pSerializedRootSig->GetBufferPointer(),
		pSerializedRootSig->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	// init slot mapping
	initMapping(rootParameters, numRootParameters);
	// get flushSize
	for (auto i = 0; i < numDescriptorTables; ++i) {
		maxFlushSize += descTables[i].size;
	}
	// clear
	if (pSerializedRootSig) {
		pSerializedRootSig->Release();
	}
	if (pError) {
		pError->Release();
	}
}

void D3D12RootSignature::initDescriptorTableCache(int tableIndex, int rootParameterIndex, D3D12_ROOT_PARAMETER1* rootParameter) {
	// get cache
	auto& descTable = descTables[tableIndex];
	// init descriptor table cache
	descTable.dirty = false;
	descTable.rootSlot = rootParameterIndex;
	descTable.descriptorType = rootParameter->DescriptorTable.pDescriptorRanges[0].RangeType;
	memset(descTable.stales, -1, sizeof(bool) * max_descriptor_table_size);
	// init all descriptor handle to null
	for (int n = 0; n < max_descriptor_table_size; n++) {
		descTable.resourceId[n] = -1;
		descTable.handles[n] = CD3DX12_CPU_DESCRIPTOR_HANDLE();
	}
	descTable.size = rootParameter->DescriptorTable.pDescriptorRanges[0].NumDescriptors;
}

void D3D12RootSignature::initMapping(D3D12_ROOT_PARAMETER1* rootParameters, int numRootParameters) {
	int i = 0;
	int offset = 0;
	int tableIndex = 0;

	for (int i = 0; i < numRootParameters; i++) {

		if (rootParameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
			// tables
			if (rootParameters[i].DescriptorTable.pDescriptorRanges[0].RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SRV) {
				// srv tables
				for (unsigned int index = 0; index < rootParameters[i].DescriptorTable.pDescriptorRanges[0].NumDescriptors; index++) {
					auto slot = rootParameters[i].DescriptorTable.pDescriptorRanges[0].BaseShaderRegister + index;
					srvs[slot].tableIndex = tableIndex;
					srvs[slot].offset = index;
					srvs[slot].rootSlot = i;
				}
				// init table cache
				initDescriptorTableCache(tableIndex, i, &rootParameters[i]);
				// increment cached tables count
				++numDescriptorTables;
			}
			else if (rootParameters[i].DescriptorTable.pDescriptorRanges[0].RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_UAV) {
				// uva tables
				for (unsigned int index = 0; index < rootParameters[i].DescriptorTable.pDescriptorRanges[0].NumDescriptors; index++) {
					auto slot = rootParameters[i].DescriptorTable.pDescriptorRanges[0].BaseShaderRegister + index;
					uavs[slot].tableIndex = tableIndex;
					uavs[slot].offset = index;
					uavs[slot].rootSlot = i;
				}
				// init table cache
				initDescriptorTableCache(tableIndex, i, &rootParameters[i]);
				// increment cached tables count
				++numDescriptorTables;
			}
			else if (rootParameters[i].DescriptorTable.pDescriptorRanges[0].RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER) {
				// sampler tables
				for (unsigned int index = 0; index < rootParameters[i].DescriptorTable.pDescriptorRanges[0].NumDescriptors; index++) {
					auto slot = rootParameters[i].DescriptorTable.pDescriptorRanges[0].BaseShaderRegister + index;
					// samplers don't use cache
					samplers[slot].tableIndex = 0;
					samplers[slot].offset = index;
					samplers[slot].rootSlot = i;
				}
				// init table cache
				initDescriptorTableCache(tableIndex, i, &rootParameters[i]);
			}
			// increase descriptor table index
			++tableIndex;
		}
		else if (rootParameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_CBV) {
			// constants buffer view
			auto slot = rootParameters[i].Descriptor.ShaderRegister;
			rootDescriptors[slot].rootSlot = i;
			rootDescriptors[slot].dirty = 0;
			// increment constant buffer view count
			++numRootDescriptors;
		}

	}
}


// create
void D3D12RootSignature::create(ID3D12Device* d3d12Device, bool local, bool compute, D3D12DescriptorHeap* nullHeap)
{
	if (!local) {
		// init graphic root signature
		CD3DX12_DESCRIPTOR_RANGE1 DescRange[5] = {};
		// texture materials src t 0-7
		DescRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 8, 0, 0);
		// texture g-buffer srv t 8-13
		DescRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 6, 8, 0);
		// texture misc srv t 14-20
		DescRange[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 7, 14, 0);
		// uavs  u 0-8
		DescRange[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 8, 0, 0);
		// samplers  s 0-2. samplers use static descriptors
		DescRange[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 3, 0); 
		/*
			b0  PerObject
			b1  PerLight
			b2  PerFrame
			b3  Animation
			b4  Misc
			b5  Not Used
			table  t0-t8
			table  t9-t13
			table  t14-t20
			table  u0-u7
			table  s0-s2
		*/
		CD3DX12_ROOT_PARAMETER1 RP[16] = {};
		// constant buffer
		RP[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE); // b0
		RP[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE); // b1
		RP[2].InitAsConstantBufferView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE); // b2
		RP[3].InitAsConstantBufferView(3, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE); // b3
		RP[4].InitAsConstantBufferView(4, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE); // b4
		RP[5].InitAsConstantBufferView(5, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE); // b5
		// tables
		RP[6].InitAsDescriptorTable(1, &DescRange[0]);
		RP[7].InitAsDescriptorTable(1, &DescRange[1]);
		RP[8].InitAsDescriptorTable(1, &DescRange[2]);
		RP[9].InitAsDescriptorTable(1, &DescRange[3]);
		RP[10].InitAsDescriptorTable(1, &DescRange[4]);

		initRootSignature(d3d12Device, false, RP, 11);
	} else {
		// init local rootsignuatre
		CD3DX12_DESCRIPTOR_RANGE1 DescRange[2] = {};
		// texture materials src t 3-10
		DescRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 2, 1);
		// uavs  u 0-8
		DescRange[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 4, 0, 1);

		CD3DX12_ROOT_PARAMETER1 RP[16]{};
		// constant buffer
		RP[0].InitAsShaderResourceView(0, 1, D3D12_ROOT_DESCRIPTOR_FLAG_NONE); // t0  vertex buffer
		RP[1].InitAsShaderResourceView(1, 1, D3D12_ROOT_DESCRIPTOR_FLAG_NONE); // t1  index buffer
		RP[2].InitAsConstants(1, 0, 1, D3D12_SHADER_VISIBILITY_ALL); // b0  
		RP[3].InitAsDescriptorTable(1, &DescRange[0]); // b1
		initRootSignature(d3d12Device, true, RP, 4);
	}
	// init null handles
	nullSRV = nullHeap->GetCpuHandle(0);
	nullUAV = nullHeap->GetCpuHandle(1);
	//
	isCompute = compute;
}

void D3D12RootSignature::SetStale() 
{
	stale = true;
	// set all tables as stale
	//for (auto i = 0; i < max_descriptor_table_num; ++i) {
	//	memset(descTables[i].stales, -1, sizeof(bool) * max_descriptor_table_size);
	//}
}

// set samplers
void D3D12RootSignature::SetSamplerTable(ID3D12GraphicsCommandList* cmdList, D3D12_GPU_DESCRIPTOR_HANDLE handle)
{
	if (!isCompute) {
		cmdList->SetGraphicsRootDescriptorTable(descTables[sampler_slot].rootSlot, handle);
	} else {
		cmdList->SetComputeRootDescriptorTable(descTables[sampler_slot].rootSlot, handle);
	}
	return;
}

void D3D12RootSignature::SetSRV(int slot, D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
	// set srv slot
	auto &tableSlot = srvs[slot];
	auto& table = descTables[tableSlot.tableIndex];
	if (table.handles[tableSlot.offset].ptr !=  handle.ptr) {
		table.handles[tableSlot.offset] = handle;
		table.dirty = true;
	}
	table.stales[tableSlot.offset] = false;
}

void D3D12RootSignature::SetUAV(int slot, D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
	// set uav slot
	auto& tableSlot = uavs[slot];
	auto& table = descTables[tableSlot.tableIndex];
	if (table.handles[tableSlot.offset].ptr != handle.ptr) {
		table.handles[tableSlot.offset] = handle;
		table.dirty = true;
	}
	table.stales[tableSlot.offset] = false;
}

void D3D12RootSignature::SetConstantBuffer(int slot, D3D12_GPU_VIRTUAL_ADDRESS buffer, unsigned int size)
{
	int alignedSize = (size + const_buffer_align - 1) & ~(const_buffer_align - 1);
	rootDescriptors[slot].constDesc.BufferLocation = buffer;
	rootDescriptors[slot].constDesc.SizeInBytes = alignedSize;
	rootDescriptors[slot].dirty = true;
}

bool D3D12RootSignature::Flush(ID3D12GraphicsCommandList* cmdList, D3D12DescriptorHeap* heap)
{
	if (!heap->HasSpace(maxFlushSize)) {
		// ensure the heap has enough space
		return false;
	}
	// flush tables
	for (auto i = 0; i < numDescriptorTables; ++i) {
		auto& table = descTables[i];
		if (table.size) {
			for (auto slot = 0; slot < table.size; ++slot) {
				if (table.stales[slot]) {
					// all tables must be flushed with null handles
					if (table.descriptorType == D3D12_DESCRIPTOR_RANGE_TYPE_SRV) {
						table.handles[slot] = nullSRV;
					}
					if (table.descriptorType == D3D12_DESCRIPTOR_RANGE_TYPE_UAV) {
						table.handles[slot] = nullUAV;
					}
				}
				// set stale
				table.stales[slot] = true;
			}
			if (table.dirty || stale) {
				auto tableGPUAddr = heap->StageDescriptors(table.handles, table.size);
				if (isCompute) {
					cmdList->SetComputeRootDescriptorTable(table.rootSlot, tableGPUAddr);
				} else {
					cmdList->SetGraphicsRootDescriptorTable(table.rootSlot, tableGPUAddr);
				}
				table.dirty = false;
			}
		}
	}
	// flush constant buffers
	for (auto i = 0; i < numRootDescriptors; ++i) {
		auto& rootDescriptor = rootDescriptors[i];
		if (rootDescriptor.dirty) {
			if (isCompute) {
				cmdList->SetComputeRootConstantBufferView(rootDescriptor.rootSlot, rootDescriptor.constDesc.BufferLocation);
			} else {
				cmdList->SetGraphicsRootConstantBufferView(rootDescriptor.rootSlot, rootDescriptor.constDesc.BufferLocation);
			}
			rootDescriptor.dirty = false;
		}
	}
	// clear stale state
	stale = false;
	return true;
}

bool D3D12RootSignature::FlushShaderBinginds(ID3D12GraphicsCommandList* cmdList, D3D12DescriptorHeap* heap, ShaderRecord* shaderRecord)
{
	if (!heap->HasSpace(maxFlushSize)) {
		// ensure the heap has enough space
		return false;
	}
	// flush tables
	for (auto i = 0; i < numDescriptorTables; ++i) {
		auto& table = descTables[i];
		if (table.size) {
			for (auto slot = 0; slot < table.size; ++slot) {
				if (table.stales[slot]) {
					// all tables must be flushed with null handles
					if (table.descriptorType == D3D12_DESCRIPTOR_RANGE_TYPE_SRV) {
						table.handles[slot] = nullSRV;
					}
					if (table.descriptorType == D3D12_DESCRIPTOR_RANGE_TYPE_UAV) {
						table.handles[slot] = nullUAV;
					}
				}
				// set stale
				table.stales[slot] = true;
			}
			if (table.dirty || stale) {
				auto tableGPUAddr = heap->StageDescriptors(table.handles, table.size);
				shaderRecord->rootParams[table.rootSlot] = tableGPUAddr.ptr;
				// set prevraytracinghandle
				table.prevRaytracingTable = tableGPUAddr;
				table.dirty = false;
			} else {
				// bindings not changed, use prev table hanle
				shaderRecord->rootParams[table.rootSlot] = table.prevRaytracingTable.ptr;
			}
		}
	}
	// flush constant buffers
	for (auto i = 0; i < numRootDescriptors; ++i) {
		auto& rootDescriptor = rootDescriptors[i];
		if (rootDescriptor.dirty) {
			shaderRecord->rootParams[rootDescriptor.rootSlot] = rootDescriptor.constDesc.BufferLocation;
			rootDescriptor.dirty = false;
		}
	}
	// clear stale state
	stale = false;
	return true;
}


/************************************************************************/
// CommandQueue
/************************************************************************/

D3D12CommandQueue* D3D12CommandQueue::queues[(unsigned int)COMMAND_CONTEXT_TYPE::COUNT] = {};

D3D12CommandQueue* D3D12CommandQueue::Alloc(ID3D12Device* d3d12Device, D3D12_COMMAND_LIST_TYPE cmdType)
{
	assert(queues[cmdType] == nullptr);
	// alloc commandQueue
	auto commandQueue = new D3D12CommandQueue();
	queues[cmdType] = commandQueue;
	// create command queue
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Type = cmdType;
	d3d12Device->CreateCommandQueue(&desc, IID_PPV_ARGS(&commandQueue->cmdQueue));
	// create fence
	d3d12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&commandQueue->cmdFence));
	// event
	// create fences, event, init fenvevalue
	for (int i = 0; i < max_render_threads; i++) {
		// event
		commandQueue->hEvents[i] = CreateEvent(0, 0, 0, 0);
	}
	commandQueue->currentFenceValue = 0;
	return nullptr;
}

void D3D12CommandQueue::GpuWait(ID3D12Fence* d3d12Fence, UINT64 fenceValue)
{
	cmdQueue->Wait(d3d12Fence, fenceValue);
}

void D3D12CommandQueue::CpuWait(UINT64 fenceValue)
{
	// get the event to wait for current thread
	size_t threadId = reinterpret_cast<size_t>(ThreadLocal::GetThreadLocal());
	cmdFence->SetEventOnCompletion(fenceValue, hEvents[threadId]);
	WaitForSingleObject(hEvents[threadId], -1);
}

bool D3D12CommandQueue::IsFenceComplete(UINT64 fenceValue)
{
	UINT64 completed = cmdFence->GetCompletedValue();
	if (fenceValue <= completed) {
		return true;
	} else {
		return false;
	}
}

UINT64 D3D12CommandQueue::ExecuteCommandList(ID3D12CommandList* cmdList)
{
	cmdQueue->ExecuteCommandLists(1, &cmdList);
	lock.Acquire();
	UINT64 newFenceValue = ++currentFenceValue;
	cmdQueue->Signal(cmdFence, newFenceValue);
	lock.Release();
	return newFenceValue;
}



/************************************************************************/
// PipelineStateCache
/************************************************************************/

D3D12PipelineStateCache::D3D12PipelineStateCache() : hash(-1), VS(-1), PS(-1), GS(-1), DS(-1), HS(-1), CS(-1), InputLayout(-1), NumRTV(0),
Depth(-1), Rasterizer(-1), Blend(-1), Dirty(0), Top(R_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED) {
	memset(RTVFormat, -1, sizeof(DXGI_FORMAT) * 8);
	DSVFormat = DXGI_FORMAT_FORCE_UINT;
}


D3D12PipelineStateCache::operator int() const {
	if (!Dirty) {
		return hash;
	}
	else {
		hash = 0;
		hash = (hash << 5) + VS;
		hash = (hash << 5) + PS;
		hash = (hash << 5) + GS;
		hash = (hash << 5) + DS;
		hash = (hash << 5) + HS;
		hash = (hash << 5) + CS;
		hash = (hash << 5) + InputLayout;
		hash = (hash << 5) + Depth;
		hash = (hash << 5) + Rasterizer;
		hash = (hash << 5) + Blend;
		return hash;
	}
}

bool D3D12PipelineStateCache::operator==(const D3D12PipelineStateCache& rh) {
	if (VS == rh.VS && PS == rh.PS && GS == rh.GS && DS == rh.DS && HS == rh.HS && CS == rh.CS) {
		if (NumRTV == rh.NumRTV && !memcmp(RTVFormat, rh.RTVFormat, sizeof(DXGI_FORMAT) * NumRTV) && DSVFormat == rh.DSVFormat && Top == rh.Top) {
			if (Depth == rh.Depth && Rasterizer == rh.Rasterizer && Blend == rh.Blend) {
				return true;
			}
		}
	}
	return false;
}

bool D3D12PipelineStateCache::operator!=(const D3D12PipelineStateCache& rh) {
	if (VS == rh.VS && PS == rh.PS && GS == rh.GS && DS == rh.DS && HS == rh.HS && CS == rh.CS) {
		if (NumRTV == rh.NumRTV && !memcmp(RTVFormat, rh.RTVFormat, sizeof(DXGI_FORMAT) * NumRTV) && DSVFormat == rh.DSVFormat && Top == rh.Top) {
			if (Depth == rh.Depth && Rasterizer == rh.Rasterizer && Blend == rh.Blend) {
				return false;
			}
		}
	}
	return true;
}



HashMap<D3D12PipelineStateCache, ID3D12PipelineState*> D3D12PipelineStateCache::psoTable;
Mutex D3D12PipelineStateCache::lock;

ID3D12PipelineState* D3D12PipelineStateCache::GetPipelineState(ID3D12Device* d3d12Device, ID3D12RootSignature* rootSignature, const D3D12PipelineStateCache& cache)
{
	// lock the table
	lock.Acquire();
	auto iter = psoTable.Find(cache);
	ID3D12PipelineState* ret;
	if (iter == psoTable.End()) {
		// create new pso
		ret = CreatePipelineState(d3d12Device, rootSignature, cache);
		psoTable[cache] = ret;
	}
	else {
		ret = *iter;
	}
	lock.Release();
	return ret;
}


ID3D12PipelineState* D3D12PipelineStateCache::CreatePipelineState(ID3D12Device* d3d12Device, ID3D12RootSignature* rootSignature, const D3D12PipelineStateCache& cache)
{
	// set a lot pso descriptions
	ID3D12PipelineState* pso = nullptr;
	if (cache.CS != -1) {
		// compute pipeline state
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
		desc.pRootSignature = rootSignature;
		desc.CS = D3D12Shader::Get(cache.CS)->ByteCode;
		desc.NodeMask = 0;
		desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		HRESULT result = d3d12Device->CreateComputePipelineState(&desc, IID_PPV_ARGS(&pso));
		if (pso == nullptr) {
			printf("CreatePso(compute) failed result:%d \n", result);
		}
	} else {
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
		desc.pRootSignature = rootSignature;
		// we only use ps and vs
		if (cache.VS != -1) {
			desc.VS = D3D12Shader::Get(cache.VS)->ByteCode;
		}
		if (cache.PS != -1) {
			desc.PS = D3D12Shader::Get(cache.PS)->ByteCode;
		}
		if (cache.Blend != -1) {
			desc.BlendState = D3D12RenderState::Get(cache.Blend)->Blend;
		}
		desc.SampleMask = 0xffffffff;
		if (cache.Rasterizer != -1) {
			desc.RasterizerState = D3D12RenderState::Get(cache.Rasterizer)->Raster;
		}
		if (cache.Depth != -1) {
			desc.DepthStencilState = D3D12RenderState::Get(cache.Depth)->Depth;
		}
		if (cache.InputLayout != -1) {
			desc.InputLayout = D3D12InputLayout::Get(cache.InputLayout)->Layout;
		}
		desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
		desc.PrimitiveTopologyType = (D3D12_PRIMITIVE_TOPOLOGY_TYPE)cache.Top;   //  D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		desc.NumRenderTargets = cache.NumRTV;
		memcpy(desc.RTVFormats, cache.RTVFormat, cache.NumRTV * sizeof(DXGI_FORMAT));
		desc.DSVFormat = cache.DSVFormat;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.NodeMask = 0;
		desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		// create it
		HRESULT result = d3d12Device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pso));
		if (pso == nullptr) {
			printf("CreatePso failed result:%d, rtvcount %d\n", result, desc.NumRenderTargets);
		}
	}
	psoTable.Set(cache, pso);
	return pso;
}


/************************************************************************/
// RTPSO
/************************************************************************/

UINT64 RaytracingStateObject::version = 0;

void RaytracingStateObject::SetStale()
{
	InterlockedIncrement(&version);
}

void RaytracingStateObject::Refresh(ID3D12Device5* rtxDevice)
{
	if (currentVersion != version) {
		// release the old one
		if (stateObject) {
			stateObject->Release();
		}
		// create a new one
		CD3DX12_STATE_OBJECT_DESC raytracingStateObject{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };
		// assume version equals shader count
		for (auto ray = 0; ray < version; ray++) {
			auto shader = RaytracingShader::Get(ray);
			auto collection = raytracingStateObject.CreateSubobject<CD3DX12_EXISTING_COLLECTION_SUBOBJECT>();
			collection->SetExistingCollection(shader->collection);
		}
		rtxDevice->CreateStateObject(raytracingStateObject, IID_PPV_ARGS(&stateObject));
		// update version
		currentVersion = version;
	}
}



/************************************************************************/
// The Rendering Interface
/************************************************************************/

D3D12RenderInterface* D3D12RenderInterface::self;

HWND D3D12RenderInterface::CreateRenderWindow(int width, int height)
{
	HWND RenderWindow = NULL;
	WNDCLASSEX wcex;
	RECT rc = { 0, 0, width, height };
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = DefWindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = NULL;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"Simple Renderer";
	wcex.hIconSm = NULL;
	RegisterClassEx(&wcex);
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	wchar_t title[1024];
	swprintf(title, 1024, L"Simple Renderer -D3D12 (%d,%d)", width, height);
	RenderWindow = CreateWindow(L"Simple Renderer", title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, NULL, NULL);
	// show this window
	ShowWindow(RenderWindow, SW_SHOW);
	UpdateWindow(RenderWindow);
	GetClientRect(RenderWindow, &rc);
	//Width = rc.right - rc.left;
	//Height = rc.bottom - rc.top;
	return RenderWindow;
}


D3D12Resource* D3D12RenderInterface::GetResource(int id)
{
	auto resourceType = (id & 0xff000000) >> 24;
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
	case static_cast<int>(D3D12Resource::RESOURCE_TYPES::BLAS):
		resource = RaytracingGeomtry::Get(id);
		break;
	}
	return resource;
}

// destory resource
void D3D12RenderInterface::DestroyResource(int id)
{
	auto resource = GetResource(id);
	resource->Release();
}

void D3D12RenderInterface::InitDescriptorHeaps()
{
	textureDescHeaps[(unsigned int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::SRV] = D3D12DescriptorHeap::Alloc(d3d12Device, max_descriptor_heap_size, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	textureDescHeaps[(unsigned int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::UAV] = D3D12DescriptorHeap::Alloc(d3d12Device, max_descriptor_heap_size, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	textureDescHeaps[(unsigned int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::RTV] = D3D12DescriptorHeap::Alloc(d3d12Device, max_descriptor_heap_size, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	textureDescHeaps[(unsigned int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::DSV] = D3D12DescriptorHeap::Alloc(d3d12Device, max_descriptor_heap_size, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	textureDescHeaps[(unsigned int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::SAMPLER] = D3D12DescriptorHeap::Alloc(d3d12Device, max_descriptor_heap_size, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	// null hanldes
	textureDescHeaps[(unsigned int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::UNBOUND] = D3D12DescriptorHeap::Alloc(d3d12Device, 16, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

	bufferDescHeaps[(unsigned int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::SRV] = D3D12DescriptorHeap::Alloc(d3d12Device, max_descriptor_heap_size, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	bufferDescHeaps[(unsigned int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::UAV] = D3D12DescriptorHeap::Alloc(d3d12Device, max_descriptor_heap_size, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	bufferDescHeaps[(unsigned int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::RTV] = D3D12DescriptorHeap::Alloc(d3d12Device, max_descriptor_heap_size, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	bufferDescHeaps[(unsigned int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::DSV] = D3D12DescriptorHeap::Alloc(d3d12Device, max_descriptor_heap_size, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	textureDescHeaps[(unsigned int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::SAMPLER] = D3D12DescriptorHeap::Alloc(d3d12Device, max_descriptor_heap_size, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
}

void D3D12RenderInterface::InitCommandQueues()
{
	D3D12CommandQueue::Alloc(d3d12Device, D3D12_COMMAND_LIST_TYPE_DIRECT);
	D3D12CommandQueue::Alloc(d3d12Device, D3D12_COMMAND_LIST_TYPE_COMPUTE);
	D3D12CommandQueue::Alloc(d3d12Device, D3D12_COMMAND_LIST_TYPE_COPY);
}

void D3D12RenderInterface::InitD3D12Device()
{
#if defined(_DEBUG)
	// Enable the D3D12 debug layer.
	{
		ID3D12Debug* debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
	}
#endif
	DXGI_QUERY_VIDEO_MEMORY_INFO videoMemoryInfo;
	// Create a DXGIFactory object.
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)))) {
		return;
	}
	IDXGIAdapter* pAdapter = nullptr;
	IDXGIAdapter3* pAdapter3 = nullptr;
	dxgiFactory->EnumAdapters(0, &pAdapter);
	pAdapter->QueryInterface(IID_PPV_ARGS(&pAdapter3));

	DXGI_ADAPTER_DESC desc{};
	pAdapter->GetDesc(&desc);
	pAdapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &videoMemoryInfo);
	// create device
	if (FAILED(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&d3d12Device)))) {
		printf("Failed to create D3D12Device\n");
		return;
	}
	// check raytracing support
	D3D12_FEATURE_DATA_D3D12_OPTIONS5  featureData{};
	D3D12_FEATURE_DATA_D3D12_OPTIONS   featureData2{};
	d3d12Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &featureData, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS5));
	d3d12Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &featureData2, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS));
	// get rtx device
	d3d12Device->QueryInterface(IID_PPV_ARGS(&rtxDevice));
	// release objects
	pAdapter->Release();
	pAdapter3->Release();
}

void D3D12RenderInterface::InitBackBuffer(int width, int height)
{
	HWND hWnd = CreateRenderWindow(width, height);
	backBuffer.Create(d3d12Device, dxgiFactory, hWnd, width, height, textureDescHeaps[static_cast<int>(D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::RTV)]);
}

void D3D12Renderer::D3D12RenderInterface::InitPrimitiveGeometry()
{
	typedef struct BasicVertex
	{
		float x, y, z;
		float u, v;
	}BasicVertex;
	// create a uniform quad geometry
	BasicVertex VBuffer[4]{};
	WORD IBuffer[6]{};
	VBuffer[0].x = 0;
	VBuffer[0].y = 1;
	VBuffer[0].z = 0;
	VBuffer[0].u = 0;// +(float)1 / Width;
	VBuffer[0].v = 0;// +(float)1 / Height;
	//2
	VBuffer[1].x = 1;
	VBuffer[1].y = 1;
	VBuffer[1].z = 0;
	VBuffer[1].u = 1;// +(float)1 / Width;
	VBuffer[1].v = 0;// +(float)1 / Height;
	//3
	VBuffer[2].x = 0;
	VBuffer[2].y = 0;
	VBuffer[2].z = 0;
	VBuffer[2].u = 0;// +(float)1 / Width;
	VBuffer[2].v = 1;// +(float)1 / Height;
	//4
	VBuffer[3].x = 1;
	VBuffer[3].y = 0;
	VBuffer[3].z = 0;
	VBuffer[3].u = 1;// +(float)1 / Width;
	VBuffer[3].v = 1;// +(float)1 / Height;
	//m_ScreenRectIndex = {0,1,2,2,1,3};
	IBuffer[0] = 0;
	IBuffer[1] = 1;
	IBuffer[2] = 2;
	IBuffer[3] = 2;
	IBuffer[4] = 1;
	IBuffer[5] = 3;
	// set QuadId for later use
	R_GEOMETRY_DESC desc{
		VBuffer,
		sizeof(BasicVertex) * 4,
		sizeof(BasicVertex),
		IBuffer,
		6,
		R_FORMAT::FORMAT_R16_UINT,
		R_PRIMITIVE_TOPOLOGY::R_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		L"FullScreenQuad",
	};
	auto quadId = CreateGeometry(&desc);
}

void D3D12RenderInterface::InitSamplers()
{
	D3D12_SAMPLER_DESC sampDesc = {};
	// anisotropic sampler
	sampDesc.Filter = D3D12_FILTER_ANISOTROPIC;// D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	sampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxAnisotropy = 16;
	sampDesc.MaxLOD = D3D12_FLOAT32_MAX;
	d3d12Device->CreateSampler(&sampDesc, textureDescHeaps[(int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::SAMPLER]->GetCpuHandle(0));
	// bilinear sampler
	sampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	d3d12Device->CreateSampler(&sampDesc, textureDescHeaps[(int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::SAMPLER]->GetCpuHandle(1));
	// point sampler
	sampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	d3d12Device->CreateSampler(&sampDesc, textureDescHeaps[(int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::SAMPLER]->GetCpuHandle(2));
}

void D3D12RenderInterface::InitNullHandles()
{
	D3D12_CPU_DESCRIPTOR_HANDLE nullHandles[2]{};
	{
		R_TEXTURE2D_DESC desc = {};
		desc.Width = 16;
		desc.Height = 16;
		desc.ArraySize = 1;
		desc.CPUAccess = (R_CPU_ACCESS)0;
		desc.BindFlag = (R_BIND_FLAG)(BIND_SHADER_RESOURCE);   // create state is srv
		desc.MipLevels = 1;
		desc.Usage = DEFAULT;
		desc.Format = FORMAT_R16G16B16A16_FLOAT;
		desc.SampleDesc.Count = 1;
		desc.DebugName = L"null-texture";
		auto id = CreateTexture2D(&desc);
		nullHandles[0] = GetResource(id)->GetSrv();
	}
	{
		R_BUFFER_DESC desc = {};
		desc.BindFlags = (R_BIND_FLAG)(BIND_UNORDERED_ACCESS);
		desc.CPUAccessFlags = (R_CPU_ACCESS)0;
		desc.Size = 16;
		desc.StructureByteStride = 16;
		desc.Usage = DEFAULT;
		desc.DebugName = L"null-buffer";
		auto id = CreateBuffer(&desc);
		nullHandles[1] = GetResource(id)->GetUav();
	}
	// copy null handles to UNBOUND heap
	auto heap = textureDescHeaps[(int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::UNBOUND];
	
	d3d12Device->CopyDescriptorsSimple(1, heap->GetCpuHandle(0), nullHandles[0], D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	d3d12Device->CopyDescriptorsSimple(1, heap->GetCpuHandle(1), nullHandles[1], D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

int D3D12RenderInterface::Initialize(int width, int height)
{
	// init self
	self = this;
	// create the device
	InitD3D12Device();
	// create decriptor heaps for resource descriptors
	InitDescriptorHeaps();
	// init queues
	InitCommandQueues();
	// init backbuffer
	InitBackBuffer(width, height);
	// init primitive geometry
	InitPrimitiveGeometry(); 
	// init samplers
	InitSamplers();
	// init null handles
	InitNullHandles();
	return 0;
}

int D3D12RenderInterface::CreateTexture2D(R_TEXTURE2D_DESC* desc)
{
	// alloc resource index
	auto resourceDesc = (ResourceDescribe *)desc;
	auto texture = TextureResource::CreateResource(d3d12Device, resourceDesc);
	// create cpu descriptors
	texture->CreateViews(d3d12Device, resourceDesc, textureDescHeaps);
	return texture->resourceId | (unsigned int)D3D12Resource::RESOURCE_TYPES::TEXTURE << 24;
}

int D3D12RenderInterface::DestroyTexture2D(int id)
{
	DestroyResource(id);
	return 0;
}

int D3D12RenderInterface::CreateBuffer(R_BUFFER_DESC* desc)
{
	// alloc resource index
	auto resourceDesc = (ResourceDescribe*)desc;
	auto buffer = BufferResource::CreateResource(d3d12Device, resourceDesc);
	// create cpu descriptors
	buffer->CreateViews(d3d12Device, resourceDesc, bufferDescHeaps);
	return buffer->resourceId | (unsigned int)D3D12Resource::RESOURCE_TYPES::BUFFER << 24;
}

int D3D12RenderInterface::DestroyBuffer(int id)
{
	DestroyResource(id);
	return 0;
}

int D3D12RenderInterface::CreateGeometry(R_GEOMETRY_DESC* desc)
{
	// alloc resource index
	auto resourceDesc = (ResourceDescribe*)desc;
	auto geometry = Geometry::CreateResource(d3d12Device, resourceDesc);
	return geometry->resourceId | (unsigned int)D3D12Resource::RESOURCE_TYPES::GEOMETRY << 24;
}

int D3D12RenderInterface::DestroyGeometry(int id)
{
	DestroyResource(id);
	return 0;
}

int D3D12Renderer::D3D12RenderInterface::CreateTransientGeometryBuffer(int geometryId)
{
	auto geometry = Geometry::Get(geometryId);
	auto transientBufferId = geometry->CreateRtGeometry(d3d12Device, true);
	return transientBufferId;
}


int D3D12RenderInterface::CreateShader(void* byteCode, unsigned int size, int flag)
{
	auto shader = D3D12Shader::Alloc();
	shader->RawCode = new char[size];
	memcpy(shader->RawCode, byteCode, size);
	shader->ByteCode.pShaderBytecode = shader->RawCode;
	shader->ByteCode.BytecodeLength = size;
	return shader->resourceId;
}


int D3D12Renderer::D3D12RenderInterface::CreateRayTracingShader(void* ByteCode, unsigned int Size, const wchar_t* Raygen, const wchar_t* Miss, const wchar_t* HitGroup, const wchar_t* ClosestHit, const wchar_t* AnyHit, const wchar_t* Intersection)
{
	auto shader = RaytracingShader::Alloc();
	// create collection
	CD3DX12_STATE_OBJECT_DESC raytracingCollection{ D3D12_STATE_OBJECT_TYPE_COLLECTION };

	auto lib = raytracingCollection.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
	D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE(ByteCode, Size);
	lib->SetDXILLibrary(&libdxil);
	// Define which shader exports to surface from the library.
	// If no shader exports are defined for a DXIL library subobject, all shaders will be surfaced.
	wchar_t clossetHitRename[128]{};
	wchar_t anyhitHitRename[128]{};
	wchar_t intersectionRename[128]{};
	wchar_t missRename[128]{};
	wchar_t raygenRename[128]{};
	{
		if (ClosestHit) {
			swprintf_s(clossetHitRename, L"%lS_%d", ClosestHit, shader->resourceId);
			lib->DefineExport(clossetHitRename, ClosestHit);
		}
		if (AnyHit) {
			swprintf_s(anyhitHitRename, L"%lS_%d", AnyHit, shader->resourceId);
			lib->DefineExport(anyhitHitRename, AnyHit);
		}
		if (Intersection) {
			swprintf_s(intersectionRename, L"%lS_%d", Intersection, shader->resourceId);
			lib->DefineExport(intersectionRename, Intersection);
		}
		if (Raygen) {
			swprintf_s(raygenRename, L"%lS_%d", Raygen, shader->resourceId);
			lib->DefineExport(raygenRename, Raygen);
		}
		if (Miss) {
			swprintf_s(missRename, L"%lS_%d", Miss, shader->resourceId);
			lib->DefineExport(missRename, Miss);
		}
	}
	// Triangle hit group
	// A hit group specifies closest hit, any hit and intersection shaders to be executed when a ray intersects the geometry's triangle/AABB.
	auto hitGroup = raytracingCollection.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
	hitGroup->SetClosestHitShaderImport(clossetHitRename);
	//hitGroup->SetAnyHitShaderImport(AnyHit);
	//hitGroup->SetIntersectionShaderImport(Intersection);
	// gen hotgroup name
	hitGroup->SetHitGroupExport(HitGroup);
	hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);
	// rs
	auto nullHeap = textureDescHeaps[(unsigned int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::UNBOUND];
	auto localRs = D3D12RootSignature::Alloc(d3d12Device, true, true, nullHeap);
	auto globalRs = D3D12RootSignature::Alloc(d3d12Device, false, true, nullHeap);
	// Add assosiations
	{
		// local rs
		auto localRootSignature = raytracingCollection.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
		localRootSignature->SetRootSignature(localRs->Get());
		// Shader association
		auto localRootSignatureAssociation = raytracingCollection.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
		localRootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
		localRootSignatureAssociation->AddExport(HitGroup);
		// global rs
		auto globalRootSignature = raytracingCollection.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
		globalRootSignature->SetRootSignature(globalRs->Get());
		// Shader association
		auto globalRootSignatureAssociation = raytracingCollection.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
		globalRootSignatureAssociation->SetSubobjectToAssociate(*globalRootSignature);
		globalRootSignatureAssociation->AddExport(raygenRename);
		globalRootSignatureAssociation->AddExport(missRename);
		globalRootSignatureAssociation->AddExport(HitGroup);
	}
	// shader config
	{
		auto shaderConfig = raytracingCollection.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
		shaderConfig->Config(sizeof(float) * 4, sizeof(float) * 2);
	}
	// pipeline config
	{
		auto pipelineConfig = raytracingCollection.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
		pipelineConfig->Config(1);
	}
	// pipeline config 
	// create collection
	rtxDevice->CreateStateObject(raytracingCollection, IID_PPV_ARGS(&shader->collection));

	ID3D12StateObjectProperties* properties;
	shader->collection->QueryInterface(IID_PPV_ARGS(&properties));
	shader->hitGroup = *(ShaderIdetifier*)properties->GetShaderIdentifier(HitGroup);
	if (Raygen) {
		shader->raygen = *(ShaderIdetifier*)properties->GetShaderIdentifier(raygenRename);
	}
	if (Miss) {
		shader->miss = *(ShaderIdetifier*)properties->GetShaderIdentifier(missRename);
	}
	// cleanup
	properties->Release();
	localRs->Release();
	globalRs->Release();
	// rtPipeline state changed
	RaytracingStateObject::SetStale();
	return shader->resourceId;
}

int D3D12RenderInterface::CreateInputLayout(R_INPUT_ELEMENT* elements, int count)
{
	auto layout = D3D12InputLayout::Alloc();
	for (int i = 0; i < count; i++) {
		D3D12_INPUT_ELEMENT_DESC& desc = layout->Element[i];
		desc.AlignedByteOffset = elements[i].Offset;
		desc.Format = (DXGI_FORMAT)elements[i].Format;
		desc.InputSlot = elements[i].Slot;
		if (elements[i].Type == R_INSTANCE) {
			desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
			desc.InstanceDataStepRate = 1;
		}
		else {
			desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			desc.InstanceDataStepRate = 0;
		}
		desc.SemanticIndex = elements[i].SemanticIndex;
		strcpy_s(layout->Names[i], elements[i].Semantic);
		desc.SemanticName = layout->Names[i];
	}
	layout->Layout.NumElements = count;
	layout->Layout.pInputElementDescs = layout->Element;
	return layout->resourceId;
}

int D3D12RenderInterface::CreateBlendStatus(R_BLEND_STATUS* Desc)
{
	auto state = D3D12RenderState::Alloc();
	state->Blend.AlphaToCoverageEnable = Desc->AlphaToCoverage;
	state->Blend.IndependentBlendEnable = FALSE;
	state->Blend.RenderTarget[0].BlendEnable = Desc->Enable;
	state->Blend.RenderTarget[0].BlendOp = (D3D12_BLEND_OP)Desc->BlendOp;
	state->Blend.RenderTarget[0].BlendOpAlpha = (D3D12_BLEND_OP)Desc->BlendOpAlpha;
	state->Blend.RenderTarget[0].DestBlend = (D3D12_BLEND)Desc->DestBlend;
	state->Blend.RenderTarget[0].DestBlendAlpha = (D3D12_BLEND)Desc->DestBlendAlpha;
	state->Blend.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	state->Blend.RenderTarget[0].LogicOpEnable = FALSE;
	state->Blend.RenderTarget[0].RenderTargetWriteMask = Desc->Mask;;
	state->Blend.RenderTarget[0].SrcBlend = (D3D12_BLEND)Desc->SrcBlend;
	state->Blend.RenderTarget[0].SrcBlendAlpha = (D3D12_BLEND)Desc->SrcBlendAlpha;
	return state->resourceId;
}

int D3D12RenderInterface::CreateDepthStencilStatus(R_DEPTH_STENCIL_DESC* Desc)
{
	auto state = D3D12RenderState::Alloc();
	// back face
	state->Depth.BackFace.StencilDepthFailOp = (D3D12_STENCIL_OP)Desc->DepthFailBack;
	state->Depth.BackFace.StencilFailOp = (D3D12_STENCIL_OP)Desc->StencilFailBack;
	state->Depth.BackFace.StencilFunc = (D3D12_COMPARISON_FUNC)Desc->StencilFuncBack;
	state->Depth.BackFace.StencilPassOp = (D3D12_STENCIL_OP)Desc->StencilPassBack;
	// front face
	state->Depth.FrontFace.StencilDepthFailOp = (D3D12_STENCIL_OP)Desc->DepthFailFront;
	state->Depth.FrontFace.StencilFailOp = (D3D12_STENCIL_OP)Desc->StencilFailFront;
	state->Depth.FrontFace.StencilFunc = (D3D12_COMPARISON_FUNC)Desc->StencilFuncFront;
	state->Depth.FrontFace.StencilPassOp = (D3D12_STENCIL_OP)Desc->StencilPassFront;
	// depth
	state->Depth.DepthEnable = Desc->ZTestEnable;
	state->Depth.DepthFunc = (D3D12_COMPARISON_FUNC)Desc->DepthFunc;
	state->Depth.DepthWriteMask = (D3D12_DEPTH_WRITE_MASK)Desc->ZWriteEnable;
	// stencil
	state->Depth.StencilEnable = Desc->StencilEnable;
	state->Depth.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	state->Depth.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

	state->StencilRef = Desc->StencilRef;
	return state->resourceId;
}

int D3D12RenderInterface::CreateRasterizerStatus(R_RASTERIZER_DESC* Desc)
{
	auto state = D3D12RenderState::Alloc();
	state->Raster.AntialiasedLineEnable = Desc->AntialiasedLineEnable;
	state->Raster.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	state->Raster.CullMode = (D3D12_CULL_MODE)Desc->CullMode;
	state->Raster.DepthBias = Desc->DepthBias;
	state->Raster.DepthBiasClamp = Desc->DepthBiasClamp;
	state->Raster.DepthClipEnable = Desc->DepthClipEnable;
	state->Raster.FillMode = (D3D12_FILL_MODE)Desc->FillMode;
	state->Raster.ForcedSampleCount = 0;
	state->Raster.FrontCounterClockwise = Desc->FrontCounterClockwise;
	state->Raster.MultisampleEnable = Desc->MultisampleEnable;
	state->Raster.SlopeScaledDepthBias = Desc->SlopeScaledDepthBias;
	return state->resourceId;
}

UINT64 D3D12RenderInterface::EndFrame(D3D12CommandContext* cmdContext)
{
	// present
	UINT64 fenceValue = backBuffer.Present(cmdContext);
	// retire all transient resource
	D3D12DescriptorHeap::RetireAll(fenceValue);
	D3D12RootSignature::RetireAll(fenceValue);
	D3D12CommandContext::RetireAll(fenceValue);
	UploadHeap::RetireAll(fenceValue);
	// retire transient rtGeometries
	RaytracingGeomtry::RetireAllTransientGeometry();
	// retire the rtScene
	RaytracingScene::RetireAll(fenceValue);
	rtScene = nullptr;
	backBuffer.WaitForNextFrame();
	QueryPerformanceFrequency(&performance.Frequency);
	QueryPerformanceCounter(&performance.EndingTime);
	performance.ElapsedMicroseconds.QuadPart = performance.EndingTime.QuadPart - performance.StartingTime.QuadPart;
	performance.StartingTime = performance.EndingTime;
	performance.ElapsedMicroseconds.QuadPart *= 1000000;
	performance.ElapsedMicroseconds.QuadPart /= performance.Frequency.QuadPart;
	char title[256] = {};
	sprintf_s(title, "Simple Renderer - D3D12(%d, %d) FPS: %lld  Draw Calls %d", backBuffer.width, backBuffer.height, 1000000 / performance.ElapsedMicroseconds.QuadPart, performance.DrawCallCount);
	SetWindowTextA(backBuffer.hWnd, title);
	performance.DrawCallCount = -1;

	return fenceValue; 
}


RenderCommandContext* D3D12RenderInterface::BeginContext(bool asyncCompute)
{
	if (asyncCompute) {
		auto cmdContext = D3D12CommandContext::AllocTransient(d3d12Device, D3D12_COMMAND_LIST_TYPE_COMPUTE, textureDescHeaps);
		return cmdContext;
	} else {
		auto cmdContext = D3D12CommandContext::AllocTransient(d3d12Device, D3D12_COMMAND_LIST_TYPE_DIRECT, textureDescHeaps);
		return cmdContext;
	}
}

UINT64 D3D12RenderInterface::EndContext(RenderCommandContext* cmdContext, bool present=false)
{
	auto d3d12Context = static_cast<D3D12CommandContext*>(cmdContext);
	if (present) {
		return EndFrame(d3d12Context);
	} else {
		return d3d12Context->Flush(false);
	}

}

RaytracingScene* D3D12RenderInterface::GetRaytracingScene()
{
	if (rtScene) {
		return rtScene;
	}
	rtScene = RaytracingScene::AllocTransient(d3d12Device);
	return rtScene;
}