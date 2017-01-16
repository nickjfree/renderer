#include "D3D12Render.h"
#include "stdio.h"
#include "DDSTextureLoader.h"
#include "DirectxMath.h"


using namespace D3D12API;
using namespace DirectX;

D3D12Render::D3D12Render() :CurrentTargets(0)
{
//	memset(Targets, 0, sizeof(void*)* 8);
}


D3D12Render::~D3D12Render()
{
}

HWND D3D12Render::CreateRenderWindow()
{
	HWND RenderWindow = NULL;
	WNDCLASSEX wcex;

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
	wcex.lpszClassName = L"H3DRender";
	wcex.hIconSm = NULL;
	RegisterClassEx(&wcex);
	RenderWindow = CreateWindowEx(0, L"H3DRender", L"H3DRender - D3D12", WS_OVERLAPPEDWINDOW, 0, 0, 1366, 768, NULL, NULL, NULL, NULL);
	// show this window
	ShowWindow(RenderWindow, SW_SHOW);
	UpdateWindow(RenderWindow);
	return RenderWindow;
}


void D3D12Render::GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter)
{
	IDXGIAdapter1 * adapter;
	*ppAdapter = nullptr;

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// Don't select the Basic Render Driver adapter.
			// If you want a software adapter, pass in "/warp" on the command line.
			continue;
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the
		// actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	*ppAdapter = adapter;
}


void D3D12Render::InitD3D12(){
	//init SwapChainDesc

#if defined(_DEBUG)
	// Enable the D3D12 debug layer.
	{
		ID3D12Debug * debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
	}
#endif
	IDXGIFactory4* pFactory = NULL;
	DXGI_QUERY_VIDEO_MEMORY_INFO videoMemoryInfo;
	// Create a DXGIFactory object.
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&pFactory)))){
		return;
	}
	pFactory->EnumAdapters(0, &pAdapter);
	pAdapter->QueryInterface(IID_PPV_ARGS(&pAdapter3));
	pAdapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &videoMemoryInfo);
	// create device
	if (FAILED(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&Device)))) {
		printf("Failed to create D3D12Device\n");
		return;
	}
	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	if (FAILED(Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&CommandQueue)))) {
		printf("Failed to create CommandQueue\n");
		return;
	}

	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = Width;
	swapChainDesc.Height = Height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	IDXGISwapChain1 * swapChain;
	if (FAILED(pFactory->CreateSwapChainForHwnd(
		CommandQueue,		// Swap chain needs the queue so that it can force a flush on it.
		hWnd,
		&swapChainDesc,
		NULL,
		NULL,
		&swapChain
	))) {
		printf("Failed to create swapchain\n");
		return;
	}

	// This sample does not support fullscreen transitions.
	pFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);
	swapChain->QueryInterface(IID_PPV_ARGS(&SwapChain));
	FrameIndex = SwapChain->GetCurrentBackBufferIndex();

	// Create descriptor heaps.
	// Describe and create a render target view (RTV) descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FrameCount;
	rtvHeapDesc.NumDescriptors = 2;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&RtvHeap));

	RtvDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// Create frame resources.
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = RtvHeap->GetCPUDescriptorHandleForHeapStart();

	// Create a RTV for each frame.
	for (UINT n = 0; n < FrameCount; n++)
	{
		SwapChain->GetBuffer(n, IID_PPV_ARGS(&RenderTargets[n]));
		Device->CreateRenderTargetView(RenderTargets[n], nullptr, rtvHandle);
		rtvHandle.ptr += RtvDescriptorSize;
	}
	// create command allocator
	Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocator));
	// test init
	// Create the command list.
	Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator, nullptr, IID_PPV_ARGS(&CommandList));

	// Command lists are created in the recording state, but there is nothing
	// to record yet. The main loop expects it to be closed, so close it now.
	CommandList->Close();

	// Create synchronization objects.
	Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence));
	FenceValue = 1;

	// Create an event handle to use for frame synchronization.
	FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (FenceEvent == nullptr) {
		printf("Create event failed %d\n", GetLastError());
	}
}

int D3D12Render::Initialize(int Width_, int Height_) {
	// Create window 
	Width = Width_;
	Height = Height_;
	hWnd = CreateRenderWindow();
	InitD3D12();
	InitShortOperation();
	return 0;
}

void D3D12Render::InitShortOperation() {
	// create a uniform quad geometry
	BasicVertex VBuffer[4];
	WORD IBuffer[6];
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
	int Id = CreateGeometry(VBuffer, sizeof(BasicVertex)* 4, sizeof(BasicVertex), IBuffer, 6, R_FORMAT::FORMAT_R16_UNORM);
}

void D3D12Render::CreateTextureDDS(D3DTexture& Texture, void * ddsData, int Size) {
}

void D3D12Render::CreateTexture2DRaw(R_TEXTURE2D_DESC* Desc, D3DTexture& texture, void * RawData, int Size) {

}

int D3D12Render::CreateTexture2D(R_TEXTURE2D_DESC* Desc, void * RawData, int Size, int DataFlag) {
	/*
		the main rendertarget texture is 0
	*/
	printf("create Texture2D\n");
	D3DTexture texture = {};
	if (Desc) {
		CreateTexture2DRaw(Desc, texture, RawData, Size);
	} else if (!Desc) {
		CreateTextureDDS(texture, RawData, Size);
	}
	int Id = Textures.AddItem(texture);
	return Id;
}


int D3D12Render::CreateGeometry(void * VBuffer, unsigned int VBSize, unsigned int VertexSize, void * IBuffer, unsigned int INum, R_FORMAT IndexFormat) {

	return 0;
}

int D3D12Render::CreateInputLayout(R_INPUT_ELEMENT * Element, int Count, void * ShaderCode, int Size) {

	return 0;
}

int D3D12Render::CreateConstantBuffer(unsigned int Size) {

	return 0;
}


int D3D12Render::CreateVertexShader(void * ByteCode, unsigned int Size, int flag) {

	return 0;
}

int D3D12Render::CreateGeometryShader(void * ByteCode, unsigned int Size, int flag) {
	return 2;
}

int D3D12Render::CreateHullShader(void * ByteCode, unsigned int Size, int flag) {
	return 3;
}

int D3D12Render::CreateDomainShader(void * ByteCode, unsigned int Size, int flag) {
	return 4;
}

int D3D12Render::CreatePixelShader(void * ByteCode, unsigned int Size, int flag) {

	return 0;
}

int D3D12Render::CreateComputeShader(void * ByteCode, unsigned int Size, int flag) {
	return 0;
}


int D3D12Render::CreateDepthStencilStatus(R_DEPTH_STENCIL_DESC* Desc) {

	return 0;
}

int D3D12Render::CreateBlendStatus(R_BLEND_STATUS* Desc) {

	return 0;
}

int D3D12Render::CreateRasterizerStatus(R_RASTERIZER_DESC* Desc) {

	return 0;
}


void D3D12Render::SetBlendStatus(int Blend) {
	if (Blend >= 0) {
		D3DRenderState& State = RenderState[Blend];
	}

}
// depth and stencil
void D3D12Render::SetDepthStencilStatus(int DepthStencil) {
	if (DepthStencil >= 0) {
		D3DRenderState& State = RenderState[DepthStencil];
	}
}
// rasterizer
void D3D12Render::SetRasterizerStatus(int Rasterizer) {

}

// viewport
void D3D12Render::SetViewPort(float tlx, float tly, float width, float height, float minz, float maxz) {

}


void D3D12Render::SetDepthStencil(int Depth) {

}

void D3D12Render::SetRenderTargets(int Count, int * Targets) {

}

void D3D12Render::SetTexture(int StartSlot, int * Texture, int Count) {

}

void D3D12Render::SetInputLayout(int Id) {

}

void D3D12Render::SetVertexShader(int Id) {
}

void D3D12Render::SetPixelShader(int Id) {

}

void D3D12Render::SetConstant(int Slot, int Buffer, void * CPUData, unsigned int Size) {

}

void D3D12Render::ClearDepth(float depth, float stencil) {
	//DeviceContext->ClearDepthStencilView(Depth, D3D12_CLEAR_DEPTH | D3D12_CLEAR_STENCIL, 1, 0);
}

void D3D12Render::ClearRenderTarget(){
	
}

void D3D12Render::Present() {
//	SwapChain->Present(1, 0);
	CommandAllocator->Reset();

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	CommandList->Reset(CommandAllocator, PipelineState);

	// Indicate that the back buffer will be used as a render target.
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Transition.pResource = RenderTargets[FrameIndex];
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	CommandList->ResourceBarrier(1, &barrier);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = RtvHeap->GetCPUDescriptorHandleForHeapStart()/*, m_rameIndex, m_rtvDescriptorSize)*/;
	rtvHandle.ptr += FrameIndex * RtvDescriptorSize;

	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	// Indicate that the back buffer will now be used to present.
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	CommandList->ResourceBarrier(1, &barrier);

	CommandList->Close();

	ID3D12CommandList* ppCommandLists[] = {CommandList};
	CommandQueue->ExecuteCommandLists(1, ppCommandLists);

	SwapChain->Present(1, 0);

	WaitForPreviousFrame();
}

void D3D12Render::WaitForPreviousFrame() {
	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
	// sample illustrates how to use fences for efficient resource usage and to
	// maximize GPU utilization.

	// Signal and increment the fence value.
	const UINT64 fence = FenceValue;
	CommandQueue->Signal(Fence, fence);
	FenceValue++;

	// Wait until the previous frame is finished.
	if (Fence->GetCompletedValue() < fence)
	{
		Fence->SetEventOnCompletion(fence, FenceEvent);
		WaitForSingleObject(FenceEvent, INFINITE);
	}
	FrameIndex = SwapChain->GetCurrentBackBufferIndex();
}

void D3D12Render::Draw(int Id) {

}

void D3D12Render::Quad() {

}