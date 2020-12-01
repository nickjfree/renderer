#include "D3D12Renderer.h"

using namespace D3D12Renderer;

/************************************************************************/
// CommandContext
/************************************************************************/

void D3D12CommandContext::create(ID3D12Device* d3d12Device, D3D12_COMMAND_LIST_TYPE cmdType)
{
	d3d12Device->CreateCommandAllocator(cmdType, IID_PPV_ARGS(&cmdAllocator));
	if (cmdAllocator) {
		d3d12Device->CreateCommandList(0, cmdType, cmdAllocator, nullptr, IID_PPV_ARGS(&cmdList));
		cmdList->QueryInterface(IID_PPV_ARGS(&rtCommandList));
	}
	this->cmdType = cmdType;
}

D3D12CommandContext* D3D12CommandContext::AllocTransient(ID3D12Device* d3d12Device, D3D12_COMMAND_LIST_TYPE cmdType)
{
	auto cmdContext = allocTransient(
		[&](D3D12CommandContext* newContext) {
			newContext->create(d3d12Device, cmdType);
		},
		[&](D3D12CommandContext* retiredContext) {
			return retiredContext->cmdType == cmdType;
		}
	);
	return cmdContext;
}

D3D12CommandContext* D3D12CommandContext::Alloc(ID3D12Device* d3d12Device, D3D12_COMMAND_LIST_TYPE cmdType)
{
	auto cmdContext = new D3D12CommandContext();
	cmdContext->create(d3d12Device, cmdType);
	return cmdContext;
}

void D3D12CommandContext::resetTransient() {
	cmdList->Reset(cmdAllocator, nullptr);
	cmdAllocator->Reset();
}

void D3D12CommandContext::Release()
{
	// free all resources
	cmdList->Reset(cmdAllocator, nullptr);
	cmdAllocator->Reset();
	cmdList->Release();
	cmdAllocator->Release();
	delete this;
}


void D3D12CommandContext::AddBarrier(D3D12_RESOURCE_BARRIER& barrier)
{
	barriers.PushBack(barrier);
}

void D3D12CommandContext::applyBarriers()
{
	if (barriers.Size()) {
		cmdList->ResourceBarrier(barriers.Size(), barriers.GetData());
	}
	// TODO: invalidate current root signature 
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
	// InitCache(rootParameters, numRootParameters);
}

void D3D12RootSignature::initDescriptorTableCache(int tableIndex, int rootParameterIndex, D3D12_ROOT_PARAMETER1* rootParameter) {
	// get cache
	auto& descTable = descTables[tableIndex];
	// init descriptor table cache
	descTable.dirty = false;
	descTable.rootSlot = rootParameterIndex;
	descTable.descriptorType = rootParameter->DescriptorTable.pDescriptorRanges[0].RangeType;
	descTable.invalid = true;
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
}

// set samplers
void D3D12RootSignature::SetSamplerTable(ID3D12CommandList* cmdList, D3D12_GPU_DESCRIPTOR_HANDLE handle)
{
	return;
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
	UINT64 complete = cmdFence->GetCompletedValue();
	if (fenceValue <= complete) {
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
	descHeaps[(unsigned int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::RTV] = D3D12DescriptorHeap::Alloc(d3d12Device, max_descriptor_heap_size, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
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

int D3D12RenderInterface::Initialize(int width, int height)
{
	// create the device
	InitD3D12Device();
	// create decriptor heaps for resource descriptors
	InitDescriptorHeaps();
	// init queues
	InitCommandQueues();
	// init backbuffer
	InitBackBuffer(width, height);
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

int D3D12Renderer::D3D12RenderInterface::CreateBlendStatus(R_BLEND_STATUS* Desc)
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

int D3D12Renderer::D3D12RenderInterface::CreateDepthStencilStatus(R_DEPTH_STENCIL_DESC* Desc)
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

int D3D12Renderer::D3D12RenderInterface::CreateRasterizerStatus(R_RASTERIZER_DESC* Desc)
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
