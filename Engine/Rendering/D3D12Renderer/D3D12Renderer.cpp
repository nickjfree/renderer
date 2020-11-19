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
