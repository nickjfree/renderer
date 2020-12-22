#include "D3D12Renderer.h"

using namespace D3D12Renderer;


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
		memcpy(cpuDst, constant.m_data, size);
		// clear dirty flag
		dirty[slot] = false;
	}
}


/************************************************************************/
// CommandContext
/************************************************************************/

void D3D12CommandContext::create(ID3D12Device* d3d12Device, D3D12_COMMAND_LIST_TYPE cmdType, D3D12DescriptorHeap* samplerHeap)
{
	d3d12Device->CreateCommandAllocator(cmdType, IID_PPV_ARGS(&cmdAllocator));
	if (cmdAllocator) {
		d3d12Device->CreateCommandList(0, cmdType, cmdAllocator, nullptr, IID_PPV_ARGS(&cmdList));
		cmdList->QueryInterface(IID_PPV_ARGS(&rtCommandList));
	}
	this->cmdType = cmdType;
	this->samplerHeap = samplerHeap;
}

D3D12CommandContext* D3D12CommandContext::AllocTransient(ID3D12Device* d3d12Device, D3D12_COMMAND_LIST_TYPE cmdType, D3D12DescriptorHeap** descHeaps)
{
	auto cmdContext = allocTransient(
		[&](D3D12CommandContext* newContext) {
			newContext->create(d3d12Device, cmdType, descHeaps[(int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::SAMPLER]);
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
	cmdContext->create(d3d12Device, cmdType, nullptr);
	return cmdContext;
}

void D3D12CommandContext::bindDescriptorHeap(D3D12DescriptorHeap* heap)
{
	ID3D12DescriptorHeap* heaps[2] = { heap->Get(), samplerHeap->Get() };
	cmdList->SetDescriptorHeaps(2, heaps);
}

void D3D12CommandContext::initialize()
{
	// ensure rootsignatures
	if (!graphicsRootSignature) {
		graphicsRootSignature = D3D12RootSignature::AllocTransient(d3d12Device, false);
		cmdList->SetGraphicsRootSignature(graphicsRootSignature->Get());
	}
	if (!computeRootSignature) {
		computeRootSignature = D3D12RootSignature::AllocTransient(d3d12Device, false);
		cmdList->SetComputeRootSignature(computeRootSignature->Get());
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
	// defualt to graphics context
	SetComputeMode(false);
	SetAsyncComputeMode(false);
}

void D3D12CommandContext::SetComputeMode(bool enabled)
{
	if (isCompute != enabled) {
		// stale rootsignatures
		graphicsRootSignature->SetStale();
		computeRootSignature->SetStale();
	}
	if (isCompute) {
		cmdList->SetGraphicsRootSignature(nullptr);
		cmdList->SetComputeRootSignature(computeRootSignature->Get());
		currentRootSignature = computeRootSignature;
	} else {
		cmdList->SetGraphicsRootSignature(graphicsRootSignature->Get());
		cmdList->SetComputeRootSignature(nullptr);
		currentRootSignature = graphicsRootSignature;
	}
	isCompute = enabled;
}

void D3D12CommandContext::resetTransient() {
	// reset cmdList
	cmdList->Reset(cmdAllocator, nullptr);
	cmdAllocator->Reset();
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
	cmdList->Release();
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
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddr;
	auto cpuData = allocTransientConstantBuffer(size, &gpuAddr);
	constantCache.Upload(slot, cpuData, size);
	// TODO: set cbv to current rootsignature
}

void D3D12CommandContext::SetSRV(int slot, int resourceId)
{
	auto resource = D3D12RenderInterface::Get()->GetResource(resourceId);
	currentRootSignature->SetSRV(slot, resource->GetSrv());
	resource->SetResourceState(this, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

void D3D12CommandContext::SetUAV(int slot, int resourceId)
{
	auto resource = D3D12RenderInterface::Get()->GetResource(resourceId);
	currentRootSignature->SetSRV(slot, resource->GetUav());
	resource->SetResourceState(this, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}

void D3D12CommandContext::SetRenderTargets(int* targets, int numTargets, int depth)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handles[8] = {};
	for (auto i = 0; i < numTargets; i++) {
		DXGI_FORMAT rtvFormat{};
		if (targets[i] == 0) {
			// backbuffer
			auto backbuffer = D3D12RenderInterface::Get()->GetBackBuffer();
			handles[i] = backbuffer->GetRtv();
			// barrier
			backbuffer->SetResourceState(this, D3D12_RESOURCE_STATE_RENDER_TARGET);
			rtvFormat = backbuffer->GetResource()->GetDesc().Format;
		} else {
			// framebuffer
			auto texture = TextureResource::Get(targets[i]);
			handles[i] = texture->GetRtv();
			// barrier
			texture->SetResourceState(this, D3D12_RESOURCE_STATE_RENDER_TARGET);
			rtvFormat = texture->GetResource()->GetDesc().Format;
		}
		// set pso rtv formats
		if (pipelineStateCache.RTVFormat[i] != rtvFormat) {
			pipelineStateCache.RTVFormat[i] = rtvFormat;
			pipelineStateCache.Dirty = true;
		}
	}
	pipelineStateCache.NumRTV = numTargets;
	D3D12_CPU_DESCRIPTOR_HANDLE* depthHandle = nullptr;
	if (depth != -1) {
		auto depthBuffer = TextureResource::Get(depth);
		auto handle = depthBuffer->GetDsv();
		depthHandle = &handle;
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
	}
	cmdList->OMSetRenderTargets(numTargets, handles, false, depthHandle);
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

void D3D12CommandContext::Draw(int geometryId)
{
}

void D3D12CommandContext::DrawInstanced(int geometryId, void* instanceBuffer, unsigned int stride, unsigned int numInstances)
{
}

void D3D12CommandContext::Quad()
{
}

void D3D12CommandContext::DispatchRays(int shaderId, int width, int height)
{
}

void D3D12CommandContext::DispatchCompute(int width, int height)
{
}

void D3D12CommandContext::applyBarriers()
{
	if (barriers.Size()) {
		cmdList->ResourceBarrier(barriers.Size(), barriers.GetData());
	}
	// TODO: invalidate current root signature 
}


void D3D12CommandContext::flushState()
{
	// TODO:  
		
	// flush barriers

	// flush rootsignature

	// flush pso
}

// flush
UINT64 D3D12CommandContext::Flush(bool wait)
{
	// flush any pending barriers
	applyBarriers();
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
	size = desc.NumDescriptors;
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
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetGPUDescriptorHandleForHeapStart(), currentIndex, incrementSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHeap::GetCpuHandle(unsigned int index)
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeap->GetCPUDescriptorHandleForHeapStart(), currentIndex, incrementSize);
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

D3D12RootSignature* D3D12RootSignature::AllocTransient(ID3D12Device* d3d12Device, bool local)
{
	auto rootSignature = allocTransient(
		[&](D3D12RootSignature* rootSignature) {
			rootSignature->create(d3d12Device, local);
		},
		[&](D3D12RootSignature* rootSignature) {
			return rootSignature->local == local;
		}
		);
	return rootSignature;
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
void D3D12RootSignature::create(ID3D12Device* d3d12Device, bool local)
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
	// TODO: create null handles

}

void D3D12RootSignature::SetStale() 
{
	// set all tables as stale
	for (auto i = 0; i < max_descriptor_table_num; ++i) {
		memset(descTables[i].stales, -1, sizeof(bool) * max_descriptor_table_size);
	}
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
}

void D3D12RootSignature::SetConstantBuffer(int slot, D3D12_GPU_VIRTUAL_ADDRESS buffer, unsigned int size)
{
	rootDescriptors[slot].constDesc.BufferLocation = buffer;
	rootDescriptors[slot].constDesc.SizeInBytes = size;
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
	commandQueue->hEvent = CreateEvent(0, 0, 0, 0);
	commandQueue->currentFenceValue = 0;
	return nullptr;
}

void D3D12CommandQueue::GpuWait(ID3D12Fence* d3d12Fence, UINT64 fenceValue)
{
	cmdQueue->Wait(d3d12Fence, fenceValue);
}

void D3D12CommandQueue::CpuWait(UINT64 fenceValue)
{
	cmdFence->SetEventOnCompletion(fenceValue, hEvent);
	WaitForSingleObject(hEvent, -1);
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

D3D12PipelineStateCache::D3D12PipelineStateCache() : hash(-1), VS(-1), PS(-1), GS(-1), DS(-1), HS(-1), InputLayout(-1), NumRTV(0),
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
		hash = (hash << 5) + InputLayout;
		hash = (hash << 5) + Depth;
		hash = (hash << 5) + Rasterizer;
		hash = (hash << 5) + Blend;
		return hash;
	}
}

bool D3D12PipelineStateCache::operator==(const D3D12PipelineStateCache& rh) {
	if (VS == rh.VS && PS == rh.PS && GS == rh.GS && DS == rh.DS && HS == rh.HS) {
		if (NumRTV == rh.NumRTV && !memcmp(RTVFormat, rh.RTVFormat, sizeof(DXGI_FORMAT) * NumRTV) && DSVFormat == rh.DSVFormat && Top == rh.Top) {
			if (Depth == rh.Depth && Rasterizer == rh.Rasterizer && Blend == rh.Blend) {
				return true;
			}
		}
	}
	return false;
}

bool D3D12PipelineStateCache::operator!=(const D3D12PipelineStateCache& rh) {
	if (VS == rh.VS && PS == rh.PS && GS == rh.GS && DS == rh.DS && HS == rh.HS) {
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
	D3D12_GRAPHICS_PIPELINE_STATE_DESC Desc = {};
	Desc.pRootSignature = rootSignature;
	// we only use ps and vs
	if (cache.VS != -1) {
		Desc.VS = D3D12Shader::Get(cache.VS)->ByteCode;
	}
	if (cache.PS != -1) {
		Desc.PS = D3D12Shader::Get(cache.PS)->ByteCode;
	}
	if (cache.Blend != -1) {
		Desc.BlendState = D3D12RenderState::Get(cache.Blend)->Blend;
	}
	Desc.SampleMask = 0xffffffff;
	if (cache.Rasterizer != -1) {
		Desc.RasterizerState = D3D12RenderState::Get(cache.Rasterizer)->Raster;
	}
	if (cache.Depth != -1) {
		Desc.DepthStencilState = D3D12RenderState::Get(cache.Depth)->Depth;
	}
	if (cache.InputLayout != -1) {
		Desc.InputLayout = D3D12InputLayout::Get(cache.InputLayout)->Layout;
	}
	Desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	Desc.PrimitiveTopologyType = (D3D12_PRIMITIVE_TOPOLOGY_TYPE)cache.Top;   //  D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	Desc.NumRenderTargets = cache.NumRTV;
	memcpy(Desc.RTVFormats, cache.RTVFormat, cache.NumRTV * sizeof(DXGI_FORMAT));
	Desc.DSVFormat = cache.DSVFormat;
	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;
	Desc.NodeMask = 0;
	Desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	// create it
	ID3D12PipelineState* pso;
	HRESULT result = d3d12Device->CreateGraphicsPipelineState(&Desc, IID_PPV_ARGS(&pso));
	if (pso == nullptr) {
		printf("CreatePso failed result:%d, rtvcount %d\n", result, Desc.NumRenderTargets);
	}
	psoTable.Set(cache, pso);
	return pso;
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

void D3D12RenderInterface::InitDescriptorHeaps()
{
	descHeaps[(unsigned int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::SRV] = D3D12DescriptorHeap::Alloc(d3d12Device, max_descriptor_heap_size, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	descHeaps[(unsigned int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::UAV] = D3D12DescriptorHeap::Alloc(d3d12Device, max_descriptor_heap_size, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	descHeaps[(unsigned int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::RTV] = D3D12DescriptorHeap::Alloc(d3d12Device, max_descriptor_heap_size, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	descHeaps[(unsigned int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::DSV] = D3D12DescriptorHeap::Alloc(d3d12Device, max_descriptor_heap_size, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	descHeaps[(unsigned int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::SAMPLER] = D3D12DescriptorHeap::Alloc(d3d12Device, max_descriptor_heap_size, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
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
	backBuffer.Create(d3d12Device, dxgiFactory, hWnd, width, height, descHeaps[static_cast<int>(D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::RTV)]);
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
	d3d12Device->CreateSampler(&sampDesc, descHeaps[(int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::SAMPLER]->GetCpuHandle(0));
	// bilinear sampler
	sampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	d3d12Device->CreateSampler(&sampDesc, descHeaps[(int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::SAMPLER]->GetCpuHandle(1));
	// point sampler
	sampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	d3d12Device->CreateSampler(&sampDesc, descHeaps[(int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::SAMPLER]->GetCpuHandle(2));
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
	return 0;
}

int D3D12RenderInterface::CreateTexture2D(R_TEXTURE2D_DESC* desc)
{
	// alloc resource index
	auto resourceDesc = (ResourceDescribe *)desc;
	auto texture = TextureResource::CreateResource(d3d12Device, resourceDesc);
	// create cpu descriptors
	texture->CreateViews(d3d12Device, resourceDesc, descHeaps);
	return texture->resourceId | (unsigned int)D3D12Resource::RESOURCE_TYPES::TEXTURE << 24;
}

int D3D12RenderInterface::DestoryTexture2D(int id)
{
	DestoryResource(id);
	return 0;
}

int D3D12RenderInterface::CreateBuffer(R_BUFFER_DESC* desc)
{
	// alloc resource index
	auto resourceDesc = (ResourceDescribe*)desc;
	auto buffer = BufferResource::CreateResource(d3d12Device, resourceDesc);
	// create cpu descriptors
	buffer->CreateViews(d3d12Device, resourceDesc, descHeaps);
	return buffer->resourceId | (unsigned int)D3D12Resource::RESOURCE_TYPES::BUFFER << 24;
}

int D3D12RenderInterface::DestoryBuffer(int id)
{
	DestoryResource(id);
	return 0;
}

int D3D12RenderInterface::CreateGeometry(R_GEOMETRY_DESC* desc)
{
	// alloc resource index
	auto resourceDesc = (ResourceDescribe*)desc;
	auto geometry = Geometry::CreateResource(d3d12Device, resourceDesc);
	return geometry->resourceId | (unsigned int)D3D12Resource::RESOURCE_TYPES::GEOMETRY << 24;
}

int D3D12RenderInterface::DestoryGeometry(int id)
{
	DestoryResource(id);
	return 0;
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
	// TODO: retire rtScene

	return fenceValue;
}


RenderCommandContext* D3D12RenderInterface::BeginContext(bool asyncCompute)
{
	if (asyncCompute) {
		auto cmdContext = D3D12CommandContext::AllocTransient(d3d12Device, D3D12_COMMAND_LIST_TYPE_COMPUTE, descHeaps);
		cmdContext->SetAsyncComputeMode(true);
		return cmdContext;
	} else {
		auto cmdContext = D3D12CommandContext::AllocTransient(d3d12Device, D3D12_COMMAND_LIST_TYPE_DIRECT, descHeaps);
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