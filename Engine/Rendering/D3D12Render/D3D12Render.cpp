#include "D3D12Render.h"
#include "stdio.h"
#include "DirectxMath.h"
#include "DDSTextureLoader12.h"



using namespace D3D12API;
using namespace DirectX;

D3D12Render * D3D12Render::thisRender = NULL;

D3D12Render::D3D12Render() : CurrentConstHeap(0), CurrentSRVHeap(0), BarrierFlushed(0)
{
//	memset(Targets, 0, sizeof(void*)* 8);
	thisRender = this;
	memset(CurrentTargets, -1, 8 * sizeof(int));
	for (int i = 0; i < NUM_FRAMES; i++) {
		PrevFenceValue[i] = 0;
	}
	// init perfomence conter preq
	QueryPerformanceFrequency(&Frequency);

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
	wchar_t title[1024];
	swprintf(title,1024, L"H3DRender -D3D12 (%d,%d)", Width, Height);
	RenderWindow = CreateWindow(L"H3DRender", title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, NULL, NULL);
	// show this window
	ShowWindow(RenderWindow, SW_SHOW);
	UpdateWindow(RenderWindow);
	GetClientRect(RenderWindow, &rc);
	//Width = rc.right - rc.left;
	//Height = rc.bottom - rc.top;
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


void D3D12Render::InitD3D12() {
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
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&pFactory)))) {
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
	InitSamplers();
	// get graphic queue
	ID3D12CommandQueue * Queue = GetQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)->Get();
	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = 2;
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

	// create resource 0 as backbuffer
	D3DTexture NewTexture = {};
	int Id = Textures.AddItem(NewTexture);
	int HeapIndex = Id / MAX_DESCRIPTOR_SIZE;
	int HeapSlot = Id % MAX_DESCRIPTOR_SIZE;
	D3DTexture& texture = Textures.GetItem(Id);
	texture.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	texture.MultiFrame = 1;
	texture.MultiResource = 1;
	for (UINT n = 0; n < NUM_FRAMES; n++) {
		SwapChain->GetBuffer(n, IID_PPV_ARGS(&RenderTargets[n]));
		rtvHandle = CpuRTVHeaps[n][HeapIndex]->GetCpuHandle(HeapSlot);
		Device->CreateRenderTargetView(RenderTargets[n], nullptr, rtvHandle);
		texture.Target[n] = rtvHandle;
		texture.Texture[n] = RenderTargets[n];
		texture.State[n].CurrentState = D3D12_RESOURCE_STATE_PRESENT;
	}
	// init rootsigature
	InitNullTexture();
	InitRootSignature();
	// init current commandcontext
	SwapCommandContext();
}

void D3D12Render::InitQueues() {
	// create 3 command queues engine
	CommandQueues[D3D12_COMMAND_LIST_TYPE_DIRECT] = new CommandQueue(Device, D3D12_COMMAND_LIST_TYPE_DIRECT);
	CommandQueues[D3D12_COMMAND_LIST_TYPE_BUNDLE] = 0;
	CommandQueues[D3D12_COMMAND_LIST_TYPE_COMPUTE] = new CommandQueue(Device, D3D12_COMMAND_LIST_TYPE_COMPUTE);
	CommandQueues[D3D12_COMMAND_LIST_TYPE_COPY] =  new CommandQueue(Device, D3D12_COMMAND_LIST_TYPE_COPY);
}

void D3D12Render::InitDescriptorHeaps() {
	for (int i = 0; i < NUM_FRAMES; i++) {
		// create cpu srv heaps
		int HeapNum = MAX_TEXTURE_SIZE / MAX_DESCRIPTOR_SIZE;
		while (HeapNum--) {
			DescriptorHeap * Heap = DescriptorHeap::Alloc(Device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
			CpuSRVHeaps[i].PushBack(Heap);
		}
		// create render target heaps
		DescriptorHeap * Heap = DescriptorHeap::Alloc(Device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
		CpuRTVHeaps[i].PushBack(Heap);
		// create depth stencil heaps
		Heap = DescriptorHeap::Alloc(Device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
		CpuDSVHeaps[i].PushBack(Heap);
	}
	// create sampler heaps
	DescriptorHeap * Heap = DescriptorHeap::Alloc(Device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	GpuSamplerHeaps.PushBack(Heap);
	// create null heaps
	NullHeap = DescriptorHeap::Alloc(Device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
}

void D3D12Render::InitRootSignature() {
	D3DTexture& texture = Textures.GetItem(NullId);
	D3D12_CPU_DESCRIPTOR_HANDLE nullHandle = texture.Resource[0];
	RootSig = new RootSignature(Device, nullHandle);
}

void D3D12Render::InitSamplers() {
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
	Device->CreateSampler(&sampDesc, GpuSamplerHeaps[0]->GetCpuHandle(0));
	// bilinear sampler
	sampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	Device->CreateSampler(&sampDesc, GpuSamplerHeaps[0]->GetCpuHandle(1));
	// point sampler
	sampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	Device->CreateSampler(&sampDesc, GpuSamplerHeaps[0]->GetCpuHandle(2));
}

void D3D12Render::InitNullTexture() {
	R_TEXTURE2D_DESC desc = {};
	desc.Width = 16;
	desc.Height = 16;
	desc.ArraySize = 1;
	desc.CPUAccess = (R_CPU_ACCESS)0;
	desc.BindFlag = (R_BIND_FLAG)(BIND_SHADER_RESOURCE|BIND_RENDER_TARGET);
	desc.MipLevels = 1;
	desc.Usage = DEFAULT;
	desc.Format = FORMAT_R16G16B16A16_FLOAT;
	desc.SampleDesc.Count = 1;
	NullId = CreateTexture2D(&desc, NULL, 0, 0);
	// change state
	D3DTexture& texture = Textures.GetItem(NullId);
	D3D12_CPU_DESCRIPTOR_HANDLE handle = texture.Resource[0];
	D3D12_RESOURCE_STATES OldState = texture.State[0].CurrentState;
	ID3D12Resource * Resource = texture.Texture[0];
	D3D12_RESOURCE_STATES NewState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(Resource, OldState, NewState);
	ResourceBarriers.PushBack(Barrier);
	texture.State[0].CurrentState = NewState;
}

int D3D12Render::Initialize(int Width_, int Height_) {
	// Create window 
	Width = Width_;
	Height = Height_;
	ViewPortWidth = Width_;
	ViewPortHeight = Height_;
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
	// set QuadId for later use
	QuadId = CreateGeometry(VBuffer, sizeof(BasicVertex)* 4, sizeof(BasicVertex), IBuffer, 6, R_FORMAT::FORMAT_R16_UINT);
}

void D3D12Render::CreateTextureDDS(D3DTexture& Texture, void * ddsData, int Size, bool * isCube) {
	printf("%s\n", __func__);
	std::vector<D3D12_SUBRESOURCE_DATA > subresources;
	HRESULT result = LoadDDSTextureFromMemory(Device, (uint8_t*)ddsData, Size, &Texture.Texture[0], subresources, 0, NULL, isCube);
	ID3D12Resource * TexResource = Texture.Texture[0];
		
	// command context
	CommandContext * Context = CommandContext::Alloc(Device, D3D12_COMMAND_LIST_TYPE_DIRECT);
	ID3D12Resource * UploadHeap;
	Context->InitializeTexture(TexResource, subresources, &UploadHeap);
	Context->Finish(1);
	UploadHeap->Release();
	Texture.State[0].CurrentState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
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
	texture.MultiResource = false;
	D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COPY_DEST;
	D3D12_CLEAR_VALUE clear = {};
	clear.Format = (DXGI_FORMAT)Desc->Format;
	if (Desc->BindFlag & BIND_DEPTH_STENCIL) {
		// create multi rt  and ds for each frame
		// Num = NUM_FRAMES;
		texture.MultiFrame = true;
		textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		// clearvalue
		clear.DepthStencil.Depth = 1;
		clear.DepthStencil.Stencil = 0;
		if (clear.Format == FORMAT_R32_TYPELESS) {
			clear.Format = (DXGI_FORMAT)FORMAT_D32_FLOAT;
		}
	} else if (Desc->BindFlag & BIND_RENDER_TARGET) {
		// Num = NUM_FRAMES;
		texture.MultiFrame = true;
		textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		state = D3D12_RESOURCE_STATE_RENDER_TARGET;
		// clearvalue
		if (clear.Format == FORMAT_R32_TYPELESS) {
			clear.Format = (DXGI_FORMAT)FORMAT_R32_FLOAT;
		}
		clear.Color[0] = 0.0f;
		clear.Color[1] = 0.0f;
		clear.Color[2] = 0.0f;
		clear.Color[3] = 0.0f;
	}
	for (int i = 0; i < Num; i++) {
		Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&textureDesc,
			state,
			&clear,
			IID_PPV_ARGS(&texture.Texture[i]));
		texture.State[i].CurrentState = state;
	}
}

int D3D12Render::CreateTexture2D(R_TEXTURE2D_DESC* Desc, void * RawData, int Size, int DataFlag) {
	/*
		the main rendertarget texture is 0
	*/
	printf("create Texture2D\n");
	D3DTexture texture = {};
	int Id = Textures.AddItem(texture);
	bool isCube;
	if (Desc) {
		CreateTexture2DRaw(Desc, texture, RawData, Size);
	} else if (!Desc) {
		CreateTextureDDS(texture, RawData, Size, &isCube);
	}
	//create descriptors in cpu descriptor heaps
	int HeapSlot = Id % MAX_DESCRIPTOR_SIZE;
	int HeapIndex = Id / MAX_DESCRIPTOR_SIZE;
	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	D3D12_SHADER_RESOURCE_VIEW_DESC vdesc = {};
	vdesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	int NumFrames = 1;
	if (Desc && Desc->BindFlag & BIND_RENDER_TARGET) {
		NumFrames = NUM_FRAMES;
		D3D12_RENDER_TARGET_VIEW_DESC rtDesc = {};
		rtDesc.Format = (DXGI_FORMAT)Desc->Format;
		rtDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtDesc.Texture2D.MipSlice = 0;
		vdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		vdesc.Texture2D.MipLevels = 1;
		vdesc.Format = (DXGI_FORMAT)Desc->Format;
		texture.RTVFormat = rtDesc.Format;
		if (Desc->Format == FORMAT_R32_TYPELESS) {
			vdesc.Format = (DXGI_FORMAT)FORMAT_R32_FLOAT;
		}
		for (int i = 0; i < NumFrames; i++) {
			handle = CpuRTVHeaps[i][HeapIndex]->GetCpuHandle(HeapSlot);
			texture.Target[i] = handle;
			Device->CreateRenderTargetView(texture.Texture[0], &rtDesc, handle);
		}
	} else if (Desc && Desc->BindFlag & BIND_DEPTH_STENCIL) {
		NumFrames = NUM_FRAMES;
		D3D12_DEPTH_STENCIL_VIEW_DESC dsDesc = {};
		dsDesc.Format = (DXGI_FORMAT)Desc->Format;
		if (Desc->Format == FORMAT_R32_TYPELESS) {
			vdesc.Format = (DXGI_FORMAT)FORMAT_R32_FLOAT;
			dsDesc.Format = (DXGI_FORMAT)FORMAT_D32_FLOAT;
		}
		vdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		vdesc.Texture2D.MipLevels = 1;
		dsDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsDesc.Texture2D.MipSlice = 0;
		texture.DSVFormat = dsDesc.Format;
		for (int i = 0; i < NumFrames; i++) {
			handle = CpuDSVHeaps[i][HeapIndex]->GetCpuHandle(HeapSlot);
			texture.Depth[i] = handle;
			Device->CreateDepthStencilView(texture.Texture[0], &dsDesc, handle);
		}
	} else if (!Desc) {
		// commen textures, only use frame 0 heaps
		NumFrames = 1;
		D3D12_RESOURCE_DESC resDesc = texture.Texture[0]->GetDesc();
		vdesc.Format = (DXGI_FORMAT)resDesc.Format;
		if (isCube) {
			vdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			vdesc.TextureCube.MipLevels = resDesc.MipLevels;
		} else {
			vdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			vdesc.Texture2D.MipLevels = resDesc.MipLevels;
		}
	}
	// create srvs
	for (int i = 0; i < NumFrames; i++) {
		handle = CpuSRVHeaps[i][HeapIndex]->GetCpuHandle(HeapSlot);
		// filter out d24s8 format
		if (vdesc.Format) {
			Device->CreateShaderResourceView(texture.Texture[0], &vdesc, handle);
			texture.Resource[i] = handle;
		}
	}
	Textures[Id] = texture;
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
	ID3D12Resource * UploadHeapV, * UploadHeapI;
	Context->InitializeVetexBuffer(Geometry.VertexResource, VBuffer, VBSize,&UploadHeapV);
	Context->InitializeIndexBuffer(Geometry.IndexResource, IBuffer, INum * sizeof(WORD), &UploadHeapI);
	Context->Finish(1);
	UploadHeapV->Release();
	UploadHeapI->Release();
	// create views
	Geometry.VBV.BufferLocation = Geometry.VertexResource->GetGPUVirtualAddress();
	Geometry.VBV.SizeInBytes = VBSize;
	Geometry.VBV.StrideInBytes = VertexSize;

	Geometry.IBV.BufferLocation = Geometry.IndexResource->GetGPUVirtualAddress();
	Geometry.IBV.Format = (DXGI_FORMAT)IndexFormat;
	Geometry.IBV.SizeInBytes = INum * sizeof(WORD);
	// Add to linear buffer
	int Id = Geometries.AddItem(Geometry);
	return Id;
}

int D3D12Render::CreateInputLayout(R_INPUT_ELEMENT * Element, int Count, void * ShaderCode, int Size) {
	D3DInputLayout NewInputLayout = {};
	int id = InputLayouts.AddItem(NewInputLayout);
	D3DInputLayout& InputLayout = InputLayouts.GetItem(id);
	for (int i = 0; i < Count; i++) {
		D3D12_INPUT_ELEMENT_DESC& desc = InputLayout.Element[i];
		desc.AlignedByteOffset = Element[i].Offset;
		desc.Format = (DXGI_FORMAT)Element[i].Format;
		desc.InputSlot = Element[i].Slot;
		if (Element[i].Type == R_INSTANCE) {
			desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
			desc.InstanceDataStepRate = 1;
		} else {
			desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			desc.InstanceDataStepRate = 0;
		}
		desc.SemanticIndex = Element[i].SemanticIndex;
		strcpy_s(InputLayout.Names[i], Element[i].Semantic);
		desc.SemanticName = InputLayout.Names[i];
	}
	InputLayout.Layout.NumElements = Count;
	InputLayout.Layout.pInputElementDescs = InputLayout.Element;
	return id;
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
	D3DRenderShader Shader = {};
	Shader.RawCode = new char[Size];
	memcpy(Shader.RawCode, ByteCode, Size);
	Shader.ByteCode.pShaderBytecode = Shader.RawCode;
	Shader.ByteCode.BytecodeLength = Size;
	int id = Shaders.AddItem(Shader);
	return id;
}

int D3D12Render::CreateGeometryShader(void * ByteCode, unsigned int Size, int flag) {
	D3DRenderShader Shader = {};
	Shader.RawCode = new char[Size];
	memcpy(Shader.RawCode, ByteCode, Size);
	Shader.ByteCode.pShaderBytecode = Shader.RawCode;
	Shader.ByteCode.BytecodeLength = Size;
	int id = Shaders.AddItem(Shader);
	return id;
}

int D3D12Render::CreateHullShader(void * ByteCode, unsigned int Size, int flag) {
	D3DRenderShader Shader = {};
	Shader.RawCode = new char[Size];
	memcpy(Shader.RawCode, ByteCode, Size);
	Shader.ByteCode.pShaderBytecode = Shader.RawCode;
	Shader.ByteCode.BytecodeLength = Size;
	int id = Shaders.AddItem(Shader);
	return id;
}

int D3D12Render::CreateDomainShader(void * ByteCode, unsigned int Size, int flag) {
	D3DRenderShader Shader = {};
	Shader.RawCode = new char[Size];
	memcpy(Shader.RawCode, ByteCode, Size);
	Shader.ByteCode.pShaderBytecode = Shader.RawCode;
	Shader.ByteCode.BytecodeLength = Size;
	int id = Shaders.AddItem(Shader);
	return id;
}

int D3D12Render::CreatePixelShader(void * ByteCode, unsigned int Size, int flag) {
	D3DRenderShader Shader = {};
	Shader.RawCode = new char[Size];
	memcpy(Shader.RawCode, ByteCode, Size);
	Shader.ByteCode.pShaderBytecode = Shader.RawCode;
	Shader.ByteCode.BytecodeLength = Size;
	int id = Shaders.AddItem(Shader);
	return id;
}

int D3D12Render::CreateComputeShader(void * ByteCode, unsigned int Size, int flag) {
	D3DRenderShader Shader = {};
	Shader.RawCode = new char[Size];
	memcpy(Shader.RawCode, ByteCode, Size);
	Shader.ByteCode.pShaderBytecode = Shader.RawCode;
	Shader.ByteCode.BytecodeLength = Size;
	int id = Shaders.AddItem(Shader);
	return id;
}


int D3D12Render::CreateDepthStencilStatus(R_DEPTH_STENCIL_DESC* Desc) {
	D3DRenderState State = {};
	// back face
	State.Depth.BackFace.StencilDepthFailOp = (D3D12_STENCIL_OP)Desc->DepthFailBack;
	State.Depth.BackFace.StencilFailOp = (D3D12_STENCIL_OP)Desc->StencilFailBack;
	State.Depth.BackFace.StencilFunc = (D3D12_COMPARISON_FUNC)Desc->StencilFuncBack;
	State.Depth.BackFace.StencilPassOp = (D3D12_STENCIL_OP)Desc->StencilPassBack;
	// front face
	State.Depth.FrontFace.StencilDepthFailOp = (D3D12_STENCIL_OP)Desc->DepthFailFront;
	State.Depth.FrontFace.StencilFailOp = (D3D12_STENCIL_OP)Desc->StencilFailFront;
	State.Depth.FrontFace.StencilFunc = (D3D12_COMPARISON_FUNC)Desc->StencilFuncFront;
	State.Depth.FrontFace.StencilPassOp = (D3D12_STENCIL_OP)Desc->StencilPassFront;
	// depth
	State.Depth.DepthEnable = Desc->ZTestEnable;
	State.Depth.DepthFunc = (D3D12_COMPARISON_FUNC)Desc->DepthFunc;
	State.Depth.DepthWriteMask = (D3D12_DEPTH_WRITE_MASK)Desc->ZWriteEnable;
	// stencil
	State.Depth.StencilEnable = Desc->StencilEnable;
	State.Depth.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	State.Depth.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

	State.StencilRef = Desc->StencilRef;
	int id = RenderState.AddItem(State);
	return id;
}

int D3D12Render::CreateBlendStatus(R_BLEND_STATUS* Desc) {
	D3DRenderState State = {};
	State.Blend.AlphaToCoverageEnable = FALSE;
	State.Blend.IndependentBlendEnable = FALSE;
	State.Blend.RenderTarget[0].BlendEnable = Desc->Enable;
	State.Blend.RenderTarget[0].BlendOp = (D3D12_BLEND_OP)Desc->BlendOp;
	State.Blend.RenderTarget[0].BlendOpAlpha = (D3D12_BLEND_OP)Desc->BlendOpAlpha;
	State.Blend.RenderTarget[0].DestBlend = (D3D12_BLEND)Desc->DestBlend;
	State.Blend.RenderTarget[0].DestBlendAlpha = (D3D12_BLEND)Desc->DestBlendAlpha;
	State.Blend.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	State.Blend.RenderTarget[0].LogicOpEnable = FALSE;
	State.Blend.RenderTarget[0].RenderTargetWriteMask = Desc->Mask;;
	State.Blend.RenderTarget[0].SrcBlend = (D3D12_BLEND)Desc->SrcBlend;
	State.Blend.RenderTarget[0].SrcBlendAlpha = (D3D12_BLEND)Desc->SrcBlendAlpha;
	int id = RenderState.AddItem(State);
	return id;
}

int D3D12Render::CreateRasterizerStatus(R_RASTERIZER_DESC* Desc) {
	D3DRenderState State = {};
	State.Raster.AntialiasedLineEnable = Desc->AntialiasedLineEnable;
	State.Raster.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	State.Raster.CullMode = (D3D12_CULL_MODE)Desc->CullMode;
	State.Raster.DepthBias = Desc->DepthBias;
	State.Raster.DepthBiasClamp = Desc->DepthBiasClamp;
	State.Raster.DepthClipEnable = Desc->DepthClipEnable;
	State.Raster.FillMode = (D3D12_FILL_MODE)Desc->FillMode;
	State.Raster.ForcedSampleCount = 0;
	State.Raster.FrontCounterClockwise = Desc->FrontCounterClockwise;
	State.Raster.MultisampleEnable = Desc->MultisampleEnable;
	State.Raster.SlopeScaledDepthBias = Desc->SlopeScaledDepthBias;
	int id = RenderState.AddItem(State);
	return id;
}


void D3D12Render::SetBlendStatus(int Blend) {
	if (Blend >= 0) {
		D3DRenderState& State = RenderState[Blend];
		if (CurrentPSO.Blend != Blend) {
			CurrentPSO.Dirty = 1;
			CurrentPSO.Blend = Blend;
		}
	}

}
// depth and stencil
void D3D12Render::SetDepthStencilStatus(int DepthStencil) {
//	printf("Set depth srencil %d\n", DepthStencil);
	if (DepthStencil >= 0) {
		D3DRenderState& State = RenderState[DepthStencil];
		if (CurrentPSO.Depth != DepthStencil) {
			CurrentPSO.Dirty = 1;
			CurrentPSO.Depth = DepthStencil;
		}
		// set stencilref
		CurrentCommandContext->GetGraphicsCommandList()->OMSetStencilRef(State.StencilRef);
	}
}
// rasterizer
void D3D12Render::SetRasterizerStatus(int Rasterizer) {
	if (Rasterizer >= 0) {
		if (CurrentPSO.Rasterizer != Rasterizer) {
			CurrentPSO.Dirty = 1;
			CurrentPSO.Rasterizer = Rasterizer;
		}
	}
}

// viewport
void D3D12Render::SetViewPort(float tlx, float tly, float width, float height, float minz, float maxz) {
	ID3D12GraphicsCommandList * cmdList = CurrentCommandContext->GetGraphicsCommandList();
	D3D12_VIEWPORT viewport;
	viewport.MinDepth = minz;
	viewport.MaxDepth = maxz;
	viewport.TopLeftX = tlx;
	viewport.TopLeftY = tly;
	viewport.Width = width;
	viewport.Height = height;
	cmdList->RSSetViewports(1, &viewport);
	// setScissorRects
	D3D12_RECT rect = {};
	rect.left = 0;
	rect.right = width;
	rect.top = 0;
	rect.bottom = height;
	cmdList->RSSetScissorRects(1, &rect);
	ViewPortWidth = width;
	ViewPortHeight = height;
}


void D3D12Render::SetDepthStencil(int Depth) {
	if (Depth == -1) {
		return;
	}
	D3DTexture& texture = Textures.GetItem(Depth);
	if (CurrentPSO.DSVFormat != texture.DSVFormat) {
		CurrentPSO.Dirty = 1;
		CurrentPSO.DSVFormat = texture.DSVFormat;
	}
	int ResourceIndex = 0;
	int HandleIndex = 0;
	if (texture.MultiFrame) {
		HandleIndex = FrameIndex;
	}
	if (texture.MultiResource) {
		ResourceIndex = FrameIndex;
	}
	D3D12_CPU_DESCRIPTOR_HANDLE handle = texture.Depth[HandleIndex];
	D3D12_RESOURCE_STATES OldState = texture.State[ResourceIndex].CurrentState;
	ID3D12Resource * Resource = texture.Texture[ResourceIndex];
	D3D12_RESOURCE_STATES NewState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	if (OldState != NewState) {
		CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(Resource, OldState, NewState);
		ResourceBarriers.PushBack(Barrier);
		texture.State[ResourceIndex].CurrentState = NewState;
	}
	this->Depth = handle;
	TargetDirty = 1;
}

void D3D12Render::SetRenderTargets(int Count, int * Targets) {
//	printf("set rendertarget %d %d\n", Count, Targets[0]);
	CurrentPSO.NumRTV = Count;
	NumTargets = Count;
	for (int i = 0; i < Count; i++) {
		CurrentTargets[i] = Targets[i];
		D3DTexture& texture = Textures.GetItem(Targets[i]);
		if (CurrentPSO.RTVFormat[i] != texture.RTVFormat) {
			CurrentPSO.RTVFormat[i] = texture.RTVFormat;
			CurrentPSO.Dirty = 1;
		}
		this->Targets[i] = texture.Target[FrameIndex];
		// translate state
		int ResourceIndex = 0;
		int HandleIndex = 0;
		if (texture.MultiFrame) {
			HandleIndex = FrameIndex;
		}
		if (texture.MultiResource) {
			ResourceIndex = FrameIndex;
		}
		D3D12_CPU_DESCRIPTOR_HANDLE handle = texture.Resource[HandleIndex];
		D3D12_RESOURCE_STATES OldState = texture.State[ResourceIndex].CurrentState;
		ID3D12Resource * Resource = texture.Texture[ResourceIndex];
		D3D12_RESOURCE_STATES NewState = D3D12_RESOURCE_STATE_RENDER_TARGET;
		if (OldState != NewState) {
			CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(Resource, OldState, NewState);
			ResourceBarriers.PushBack(Barrier);
			texture.State[ResourceIndex].CurrentState = NewState;
		}
	}
	TargetDirty = 1;
}

void D3D12Render::SetTexture(int StartSlot, int * Texture, int Count) {
//	&CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	for (int i = 0; i < Count; i++) {
		int Slot = StartSlot + i;
		int Id = Texture[i];
		D3DTexture& texture = Textures.GetItem(Id);
		int ResourceIndex = 0;
		int HandleIndex = 0;
		if (texture.MultiFrame) {
			HandleIndex = FrameIndex;
		}
		if (texture.MultiResource) {
			ResourceIndex = FrameIndex;
		}
		D3D12_CPU_DESCRIPTOR_HANDLE handle = texture.Resource[HandleIndex];
		D3D12_RESOURCE_STATES OldState = texture.State[ResourceIndex].CurrentState;
		ID3D12Resource * Resource = texture.Texture[ResourceIndex];
		D3D12_RESOURCE_STATES NewState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE|D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		// bind to root signature
		bool dirty = RootSig->SetTexture(Slot, Id, handle);
		if (dirty && OldState != NewState) {
			CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(Resource, OldState, NewState);
			ResourceBarriers.PushBack(Barrier);
			texture.State[ResourceIndex].CurrentState = NewState;
		}
	}
}

void D3D12Render::SetInputLayout(int Id) {
	if (CurrentPSO.InputLayout != Id) {
		CurrentPSO.Dirty = 1;
		CurrentPSO.InputLayout = Id;
	}
}

void D3D12Render::SetVertexShader(int Id) {
	if (CurrentPSO.VS != Id) {
		CurrentPSO.Dirty = 1;
		CurrentPSO.VS = Id;
	}
}

void D3D12Render::SetPixelShader(int Id) {
	if (CurrentPSO.PS != Id) {
		CurrentPSO.Dirty = 1;
		CurrentPSO.PS = Id;
	}
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
	} else {
		CurrentConstHeap = Heap::Alloc(Device, Heap::HeapType::CPU);
		Data = CurrentConstHeap->SubAlloc(Size);
	}
	assert(Data);
	memcpy(Data, CPUData, Size);
	// bind to signature
	D3D12_GPU_VIRTUAL_ADDRESS GpuAddr = CurrentConstHeap->GetGpuAddress(Data);
	D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
	desc.BufferLocation = GpuAddr;
	desc.SizeInBytes = Size;
	bool Dirty = RootSig->SetConstantBuffer(Slot, &desc);
}


void D3D12Render::ClearDepth(float depth, float stencil) {
	FlushResourceBarriers();
	CurrentCommandContext->GetGraphicsCommandList()->ClearDepthStencilView(Depth, D3D12_CLEAR_FLAG_DEPTH| D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
}

void D3D12Render::ClearRenderTarget(){
	FlushResourceBarriers();
	ID3D12GraphicsCommandList * commandList = CurrentCommandContext->GetGraphicsCommandList();
	for (int i = 0; i < NumTargets; i++) {
		const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		commandList->ClearRenderTargetView(Targets[i], clearColor, 0, 0);
	}
}

void D3D12Render::Present() {
	if (!NumTargets || CurrentTargets[0]) {
		return;
	}
	CommandContext * Context = CurrentCommandContext;
	ID3D12GraphicsCommandList * commandlist = CurrentCommandContext->GetGraphicsCommandList();
	// flush resource barriers
	FlushResourceBarriers();
	// Indicate that the back buffer will be used as present.
	D3D12_RESOURCE_BARRIER barrier = {};
	// Indicate that the back buffer will now be used to present.
	barrier.Transition.pResource = RenderTargets[FrameIndex];
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	commandlist->ResourceBarrier(1, &barrier);
	D3DTexture& texture = Textures.GetItem(0);
	texture.State[FrameIndex].CurrentState = D3D12_RESOURCE_STATE_PRESENT;

	UINT64 FenceValue = Context->Finish(0);
	// retire all used constan heaps
	if (CurrentConstHeap) {
		UsedConstHeaps.PushBack(CurrentConstHeap);
		CurrentConstHeap = 0;
	}
	for (int i = 0; i < UsedConstHeaps.Size(); i++) {
		UsedConstHeaps[i]->Retire(FenceValue);
	}
	UsedConstHeaps.Empty();
	// retire all gpu srv heaps
	if (CurrentSRVHeap) {
		UsedGpuSRVHeaps.PushBack(CurrentSRVHeap);
		CurrentSRVHeap = 0;
	}
	for (int i = 0; i < UsedGpuSRVHeaps.Size(); i++) {
		UsedGpuSRVHeaps[i]->Retire(FenceValue);
	}
	UsedGpuSRVHeaps.Empty();
	SwapChain->Present(1, 0);
	// save this FenceValue for later waiting
	PrevFenceValue[FrameIndex] = FenceValue;
	// change current command context and set rootsignature
	SwapCommandContext();
	WaitForPreviousFrame();
	QueryPerformanceCounter(&EndingTime);
	ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
	StartingTime = EndingTime;
	ElapsedMicroseconds.QuadPart *= 1000000;
	ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
	sprintf_s(WindowTitle, "H3DRender - D3D12(%d, %d) FPS: %d", Width, Height, 1000000/ ElapsedMicroseconds.QuadPart);
	SetWindowTextA(hWnd, WindowTitle);
}

void D3D12Render::WaitForPreviousFrame() {
	// ring buffer command submiting. without idling the GPU
	CommandQueue * Queue = GetQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
//	Queue->IdleGpu();
	FrameIndex = SwapChain->GetCurrentBackBufferIndex();
	UINT FenceToWait = PrevFenceValue[FrameIndex];
	// wait for prev frame
	static DWORD s;
	Queue->Wait(FenceToWait);
	DWORD e = GetTickCount();
//	printf("%d\n", e - s);
	s = e;
}

ID3D12PipelineState * D3D12Render::CreatePSO(PSOCache& cache) {
	static int i = 0;
	i++;
//	printf("create %d pso\n", i);
	ID3D12PipelineState * PSO = NULL;
	// set a lot pso descriptions
	D3D12_GRAPHICS_PIPELINE_STATE_DESC Desc = {};
	Desc.pRootSignature = RootSig->Get();
	// we only use ps and vs
	if (cache.VS != -1) {
		Desc.VS = Shaders.GetItem(cache.VS).ByteCode;
	} 
	if (cache.PS != -1) {
		Desc.PS = Shaders.GetItem(cache.PS).ByteCode;
	}
	if (cache.Blend != -1) {
		Desc.BlendState = RenderState.GetItem(cache.Blend).Blend;
	}
	Desc.SampleMask = 0xffffffff;
	if (cache.Rasterizer != -1) {
		Desc.RasterizerState = RenderState.GetItem(cache.Rasterizer).Raster;
	}
	if (cache.Depth != -1) {
		Desc.DepthStencilState = RenderState.GetItem(cache.Depth).Depth;
	}
	if (cache.InputLayout != -1) {
		Desc.InputLayout = InputLayouts.GetItem(cache.InputLayout).Layout;
	}
	Desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	Desc.NumRenderTargets = cache.NumRTV;
	memcpy(Desc.RTVFormats, cache.RTVFormat, cache.NumRTV * sizeof(DXGI_FORMAT));
	Desc.DSVFormat = cache.DSVFormat;
	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;
	Desc.NodeMask = 0;
	Desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	// create it
	HRESULT result = Device->CreateGraphicsPipelineState(&Desc, IID_PPV_ARGS(&PSO));
	PSOTable.Set(cache, PSO);
	return PSO;
}

void D3D12Render::SwapCommandContext() {
	CurrentCommandContext = CommandContext::Alloc(Device, D3D12_COMMAND_LIST_TYPE_DIRECT);
	ID3D12GraphicsCommandList * cmdList = CurrentCommandContext->GetGraphicsCommandList();
	cmdList->SetGraphicsRootSignature(RootSig->Get());
	// set primitive topology to triangleist
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// clear targets and depth
	NumTargets = 0;
	TargetDirty = 0;
	// reset viewport
	SetViewPort(0, 0, ViewPortWidth, ViewPortHeight, 0, 1);
}


void D3D12Render::FlushRootSignature() {
	ID3D12GraphicsCommandList * cmdList = CurrentCommandContext->GetGraphicsCommandList();
	ID3D12DescriptorHeap * Heaps[2];
	Heaps[1] = GpuSamplerHeaps[0]->Get();
	int HeapChanged = 0;
	if (!CurrentSRVHeap) {
		CurrentSRVHeap = DescriptorHeap::Alloc(Device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		Heaps[0] = CurrentSRVHeap->Get();
		HeapChanged = 1;
		cmdList->SetDescriptorHeaps(2, Heaps);
		// set sampler descriptor table
		RootSig->SetSamplerTable(cmdList, GpuSamplerHeaps[0]->GetGpuHandle(0));
	}
	
	if (!RootSig->Flush(cmdList, CurrentSRVHeap, BarrierFlushed)) {
		// need new heaps
		UsedGpuSRVHeaps.PushBack(CurrentSRVHeap);
		CurrentSRVHeap = DescriptorHeap::Alloc(Device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
		HeapChanged = 1;
		Heaps[0] = CurrentSRVHeap->Get();
		cmdList->SetDescriptorHeaps(2, Heaps);
		// set sampler descriptor table
		RootSig->SetSamplerTable(cmdList, GpuSamplerHeaps[0]->GetGpuHandle(0));
		RootSig->Flush(cmdList, CurrentSRVHeap, BarrierFlushed);
	}
	// clear barrierflushed
	BarrierFlushed = 0;
}

int D3D12Render::FlushResourceBarriers() {
	int Count = ResourceBarriers.Size();
	if (Count) {
//		printf("flush %d barriers\n", Count);
		// just flush them all
		CurrentCommandContext->GetGraphicsCommandList()->ResourceBarrier(Count, ResourceBarriers.GetData());
		ResourceBarriers.Empty();
		BarrierFlushed = 1;
	}
	return Count;
}

void D3D12Render::FlushPSO() {
	ID3D12GraphicsCommandList * cmdList = CurrentCommandContext->GetGraphicsCommandList();
	if (CurrentPSO.Dirty) {
		HashMap<PSOCache, ID3D12PipelineState *>::Iterator Iter;
		Iter = PSOTable.Find(CurrentPSO);
		ID3D12PipelineState * PSO;
		if (Iter == PSOTable.End()) {
			// create new pso
			PSO = CreatePSO(CurrentPSO);
			PSOTable[CurrentPSO] = PSO;
		} else {
			PSO = *Iter;
		}
		// set pso
		cmdList->SetPipelineState(PSO);
	
	}
	CurrentPSO.Dirty = 0;	
}

void D3D12Render::FlushRenderTargets() {
	// set targets and depthstencil
	if (TargetDirty) {
		ID3D12GraphicsCommandList * cmdList = CurrentCommandContext->GetGraphicsCommandList();
		cmdList->OMSetRenderTargets(NumTargets, Targets, 0, &Depth);
		TargetDirty = 0;
	}
}

void D3D12Render::Draw(int Id) {
	FlushResourceBarriers();
	FlushRootSignature();
	FlushPSO();
	FlushRenderTargets();
	// draw
	ID3D12GraphicsCommandList * cmdList = CurrentCommandContext->GetGraphicsCommandList();
	D3DGeometry &Geometry = Geometries.GetItem(Id);
	cmdList->IASetVertexBuffers(0, 1, &Geometry.VBV);
	cmdList->IASetIndexBuffer(&Geometry.IBV);
	cmdList->DrawIndexedInstanced(Geometry.INum, 1, 0, 0, 0);
//	printf("draw %d\n", Geometry.INum);
}

void D3D12Render::DrawInstance(int Id, void * InstanceBuffer, unsigned int BufferSize, unsigned int InstanceNum) {
	FlushResourceBarriers();
	FlushRootSignature();
	FlushPSO();
	FlushRenderTargets();

	unsigned int Size = BufferSize * InstanceNum;
	void * Data;
	if (CurrentConstHeap) {
		Data = CurrentConstHeap->SubAlloc(Size);
		if (!Data) {
			UsedConstHeaps.PushBack(CurrentConstHeap);
			CurrentConstHeap = Heap::Alloc(Device, Heap::HeapType::CPU);
			Data = CurrentConstHeap->SubAlloc(Size);
		}
	}
	else {
		CurrentConstHeap = Heap::Alloc(Device, Heap::HeapType::CPU);
		Data = CurrentConstHeap->SubAlloc(Size);
	}
	assert(Data);
	memcpy(Data, InstanceBuffer, Size);
	// bind to signature
	D3D12_GPU_VIRTUAL_ADDRESS GpuAddr = CurrentConstHeap->GetGpuAddress(Data);
	D3D12_VERTEX_BUFFER_VIEW InstanceDesc[2];
	InstanceDesc[1].BufferLocation = GpuAddr;
	InstanceDesc[1].SizeInBytes = Size;
	InstanceDesc[1].StrideInBytes = BufferSize;

	ID3D12GraphicsCommandList * cmdList = CurrentCommandContext->GetGraphicsCommandList();
	D3DGeometry &Geometry = Geometries.GetItem(Id);
	InstanceDesc[0] = Geometry.VBV;
	cmdList->IASetVertexBuffers(0, 2, InstanceDesc);
	cmdList->IASetIndexBuffer(&Geometry.IBV);
	cmdList->DrawIndexedInstanced(Geometry.INum, InstanceNum, 0, 0, 0);
}

void D3D12Render::Quad() {
	FlushResourceBarriers();
	FlushRootSignature();
	FlushPSO();
	FlushRenderTargets();
	ID3D12GraphicsCommandList * cmdList = CurrentCommandContext->GetGraphicsCommandList();
	D3DGeometry &Geometry = Geometries.GetItem(QuadId);
	cmdList->IASetVertexBuffers(0, 1, &Geometry.VBV);
	cmdList->IASetIndexBuffer(&Geometry.IBV);
	cmdList->DrawIndexedInstanced(Geometry.INum, 1, 0, 0, 0);
//	printf("Quad\n");
}