#include "D3D12Render.h"
#include "stdio.h"
#include "DirectxMath.h"
#include "DDSTextureLoader12.h"



using namespace D3D12API;
using namespace DirectX;

D3D12Render * D3D12Render::thisRender = NULL;

D3D12Render::D3D12Render() :CurrentTargets(0), CurrentConstHeap(0)
{
//	memset(Targets, 0, sizeof(void*)* 8);
	thisRender = this;
}


D3D12Render::~D3D12Render() {
}


HWND D3D12Render::CreateRenderWindow()
{
	HWND RenderWindow = NULL;
	WNDCLASSEX wcex;
	RECT rc = { 0, 0, Width, Height };
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
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	RenderWindow = CreateWindow(L"H3DRender", L"H3DRender - D3D12", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, NULL, NULL);
	// show this window
	ShowWindow(RenderWindow, SW_SHOW);
	UpdateWindow(RenderWindow);
	GetClientRect(RenderWindow, &rc);
	Width = rc.right - rc.left;
	Height = rc.bottom - rc.top;
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
	// create all command queues.
	InitQueues();
	InitDescriptorHeaps();
	// get graphic queue
	ID3D12CommandQueue * Queue = GetQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)->Get();
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
		Queue,		// Swap chain needs the queue so that it can force a flush on it.
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
}


void D3D12Render::InitQueues() {
	// create 3 command queues engine
	CommandQueues[D3D12_COMMAND_LIST_TYPE_DIRECT] = new CommandQueue(Device, D3D12_COMMAND_LIST_TYPE_DIRECT);
	CommandQueues[D3D12_COMMAND_LIST_TYPE_BUNDLE] = 0;
	CommandQueues[D3D12_COMMAND_LIST_TYPE_COMPUTE] = new CommandQueue(Device, D3D12_COMMAND_LIST_TYPE_COMPUTE);
	CommandQueues[D3D12_COMMAND_LIST_TYPE_COPY] =  new CommandQueue(Device, D3D12_COMMAND_LIST_TYPE_COPY);
}

void D3D12Render::InitDescriptorHeaps() {
	int HeapNum = MAX_TEXTURE_SIZE / MAX_DESCRIPTOR_SIZE;
	while (HeapNum--) {
		DescriptorHeap * Heap = DescriptorHeap::Alloc(Device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
		CpuSRVHeaps.PushBack(Heap);
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
	printf("%s\n", __func__);
	std::vector<D3D12_SUBRESOURCE_DATA > subresources;
	HRESULT result = LoadDDSTextureFromMemory(Device, (uint8_t*)ddsData, Size, &Texture.Texture[0], subresources);
	ID3D12Resource * TexResource = Texture.Texture[0];

	// command context
	CommandContext * Context = CommandContext::Alloc(Device, D3D12_COMMAND_LIST_TYPE_DIRECT);
	Context->InitializeTexture(TexResource, subresources);
	Context->Finish(1);
}

void D3D12Render::CreateTexture2DRaw(R_TEXTURE2D_DESC* Desc, D3DTexture& texture, void * RawData, int Size) {

	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.Format = (DXGI_FORMAT)Desc->Format;
	textureDesc.Width = Desc->Width;
	textureDesc.Height = Desc->Height;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	textureDesc.MipLevels = Desc->MipLevels;
	if (RawData) {
		printf("D3d12 no suport for rawdata %s", __func__);
		return;
	}
	// try create comitted resource
	int Num = 1;
	texture.MultiFrame = false;
	if (Desc->BindFlag & BIND_DEPTH_STENCIL || Desc->BindFlag & BIND_RENDER_TARGET) {
		// create multi rt  and ds for each frame
		Num = NUM_FRAMES;
		texture.MultiFrame = true;
	}
	for (int i = 0; i < Num; i++) {
		Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&texture.Texture[i]));
		texture.State[i].CurrentState = D3D12_RESOURCE_STATE_COPY_DEST;
	}
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
	D3DGeometry Geometry = {};
	// create vertex buffer
	Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(VBSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&Geometry.VertexResource));
	// create index buffer
	Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(INum * sizeof(WORD)),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&Geometry.IndexResource));
	Geometry.VBSize = VertexSize;
	Geometry.INum = INum;
	Geometry.VSize = VBSize;
	// copy resource
	CommandContext * Context = CommandContext::Alloc(Device, D3D12_COMMAND_LIST_TYPE_DIRECT);
	Context->InitializeVetexBuffer(Geometry.VertexResource, VBuffer, VBSize);
	Context->InitializeIndexBuffer(Geometry.IndexResource, IBuffer, INum * sizeof(WORD));
	Context->Finish(1);
	// Add to linear buffer
	int Id = Geometries.AddItem(Geometry);
	return Id;
}

int D3D12Render::CreateInputLayout(R_INPUT_ELEMENT * Element, int Count, void * ShaderCode, int Size) {
	return 0;
}

int D3D12Render::CreateConstantBuffer(unsigned int Size) {
	D3DConstant Constant = {};
	Constant.Buffer = new char[Size];
	Constant.Slot = -1;
	Constant.Size = Size;
	int Id = Constants.AddItem(Constant);
	return Id;
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
	D3DConstant& Constant = Constants.GetItem(Buffer);
	void * Data = NULL;
	if (CurrentConstHeap) {
		Data = CurrentConstHeap->SubAlloc(Size);
		if (!Data) {
			UsedConstHeaps.PushBack(CurrentConstHeap);
			CurrentConstHeap = Heap::Alloc(Device, Heap::HeapType::CPU);
			Data = CurrentConstHeap->SubAlloc(Size);
		}
	} 	else {
		CurrentConstHeap = Heap::Alloc(Device, Heap::HeapType::CPU);
		Data = CurrentConstHeap->SubAlloc(Size);
	}
	assert(Data);
	memcpy(Data, CPUData, Size);
	// bind to signature
	D3D12_GPU_VIRTUAL_ADDRESS GpuAddr = CurrentConstHeap->GetGpuAddress(Data);
}


void D3D12Render::ClearDepth(float depth, float stencil) {
	//DeviceContext->ClearDepthStencilView(Depth, D3D12_CLEAR_DEPTH | D3D12_CLEAR_STENCIL, 1, 0);
}

void D3D12Render::ClearRenderTarget(){
	
}

void D3D12Render::Present() {
	CommandContext * Context = CommandContext::Alloc(Device, D3D12_COMMAND_LIST_TYPE_DIRECT);
	ID3D12GraphicsCommandList * commandlist = Context->GetGraphicsCommandList();
	// Indicate that the back buffer will be used as a render target.
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Transition.pResource = RenderTargets[FrameIndex];
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	commandlist->ResourceBarrier(1, &barrier);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = RtvHeap->GetCPUDescriptorHandleForHeapStart()/*, m_rameIndex, m_rtvDescriptorSize)*/;
	rtvHandle.ptr += FrameIndex * RtvDescriptorSize;

	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	commandlist->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	// Indicate that the back buffer will now be used to present.
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	commandlist->ResourceBarrier(1, &barrier);
	
	UINT64 FenceValue = Context->Finish(1);
	// retire all used heaps
	if (CurrentConstHeap) {
		UsedConstHeaps.PushBack(CurrentConstHeap);
		CurrentConstHeap = 0;
	}
	for (int i = 0; i < UsedConstHeaps.Size(); i++) {
		UsedConstHeaps[i]->Retire(FenceValue);
	}
	UsedConstHeaps.Empty();
	SwapChain->Present(1, 0);
	WaitForPreviousFrame();
}

void D3D12Render::WaitForPreviousFrame() {
	CommandQueue * Queue = GetQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	Queue->IdleGpu();
	FrameIndex = SwapChain->GetCurrentBackBufferIndex();
}

void D3D12Render::Draw(int Id) {
}

void D3D12Render::Quad() {

}