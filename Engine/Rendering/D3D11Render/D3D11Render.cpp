#include "D3D11Render.h"
#include "stdio.h"
#include "DDSTextureLoader.h"
#include "DirectxMath.h"


using namespace D3D11API;
using namespace DirectX;

D3D11Render::D3D11Render() :CurrentTargets(0), Depth(0)
{
	memset(Targets, 0, sizeof(void*)* 8);
}


D3D11Render::~D3D11Render()
{
}

HWND D3D11Render::CreateRenderWindow()
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
	RenderWindow = CreateWindowEx(0, L"H3DRender", L"H3DRender", WS_OVERLAPPEDWINDOW, 0, 0, 1366, 768, NULL, NULL, NULL, NULL);
	// show this window
	ShowWindow(RenderWindow, SW_SHOW);
	UpdateWindow(RenderWindow);
	return RenderWindow;
}

void D3D11Render::InitD3D11(){
	//init SwapChainDesc
	DXGI_SWAP_CHAIN_DESC SwapChainDesc;
	ZeroMemory(&SwapChainDesc, sizeof(SwapChainDesc));
	SwapChainDesc.BufferDesc.Width = Width;
	SwapChainDesc.BufferDesc.Height = Height;
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.BufferCount = 1;
	SwapChainDesc.OutputWindow = hWnd;
	SwapChainDesc.Windowed = true;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	//nVidia PerHUD
	IDXGIAdapter * pAdapter;
	IDXGIFactory* pFactory = NULL;

	// Create a DXGIFactory object.
	if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory)))
	{
		return;
	}
	DXGI_ADAPTER_DESC  AdapterDesc;
	pFactory->EnumAdapters(0, &pAdapter);
	pAdapter->GetDesc(&AdapterDesc);

	//DWORD ret = D3D11CreateDeviceAndSwapChain(pAdapter,D3D_DRIVER_TYPE_REFERENCE,0,/*D3D10_CREATE_DEVICE_DEBUG*/D3D11_CREATE_DEVICE_SINGLETHREADED,
	//	NULL,0,D3D11_SDK_VERSION,&SwapChainDesc,&m_SwapChain,&m_Device,&m_FeatureLevel,&m_DeviceContext);
	// Create Device and Swap Chain
	DWORD ret = D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0,/*D3D10_CREATE_DEVICE_DEBUG*/0,/*D3D11_CREATE_DEVICE_SINGLETHREADED*/
		NULL, 0, D3D11_SDK_VERSION, &SwapChainDesc, &SwapChain, &Device, &FeatureLevel, &DeviceContext);
	printf("D3DDeivce %x, DeviceContex %x\n", Device, DeviceContext);
	// the following code shold change to : adding a textureid 0 to textures
	ID3D11Texture2D * BackBuffer;
	D3DTexture Texture;
	SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer);
	D3D11_RENDER_TARGET_VIEW_DESC backbufferViewDesc;
	ZeroMemory(&backbufferViewDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	backbufferViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	backbufferViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
	backbufferViewDesc.Texture2D.MipSlice = 0;
	Texture.Texture = BackBuffer;
    Device->CreateRenderTargetView(BackBuffer, &backbufferViewDesc, &Texture.Target);
	Textures.AddItem(Texture);

	D3D11_VIEWPORT ViewPort;
	ViewPort.TopLeftX = 0;
	ViewPort.TopLeftY = 0;
	ViewPort.Width = Width;
	ViewPort.Height = Height;
	ViewPort.MinDepth = 0.0f;
	ViewPort.MaxDepth = 1.0f;
	DeviceContext->RSSetViewports(1, &ViewPort);

	// set default rasterizer
	ID3D11RasterizerState * State;
	D3D11_RASTERIZER_DESC Desc;
	memset(&Desc, 0, sizeof(Desc));
	// i need to convert RSDeac to D3D11_RASTERIZER_DESC,only convert strings
	Desc.FillMode = D3D11_FILL_SOLID;
	Desc.CullMode = D3D11_CULL_BACK;
	Desc.FrontCounterClockwise = 1;
	// multisample
	Desc.MultisampleEnable = true;
	Desc.AntialiasedLineEnable = true;
	Device->CreateRasterizerState(&Desc, &State);
	DeviceContext->RSSetState(State);

	// sampler
	ID3D11SamplerState * Sampler;
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	// anisotropic sampler
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;// D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	Device->CreateSamplerState(&sampDesc, &Sampler);
	DeviceContext->PSSetSamplers(0, 1, &Sampler);
	// bilinear sampler
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	Device->CreateSamplerState(&sampDesc, &Sampler);
	DeviceContext->PSSetSamplers(1, 1, &Sampler);
	// point sampler
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	Device->CreateSamplerState(&sampDesc, &Sampler);
	DeviceContext->PSSetSamplers(2, 1, &Sampler);
}

int D3D11Render::Initialize(int Width_, int Height_) {
	// Create window 
	Width = Width_;
	Height = Height_;
	hWnd = CreateRenderWindow();
	InitD3D11();
	InitShortOperation();
	return 0;
}

void D3D11Render::InitShortOperation() {
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

void D3D11Render::CreateTextureDDS(D3DTexture& Texture, void * ddsData, int Size) {
	CreateDDSTextureFromMemory(Device, (uint8_t*)ddsData, Size, &Texture.Texture, &Texture.Resource);
}

void D3D11Render::CreateTexture2DRaw(R_TEXTURE2D_DESC* Desc, D3DTexture& texture, void * RawData, int Size) {
	D3D11_TEXTURE2D_DESC desc = {};
	D3D11_SUBRESOURCE_DATA Initial, *pInitial = 0;
	if (Desc) {
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = Desc->Width;
		desc.Height = Desc->Height;
		desc.MipLevels = Desc->MipLevels;
		desc.ArraySize = Desc->ArraySize;
		//desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.Format = (DXGI_FORMAT)Desc->Format;
		desc.SampleDesc.Count = Desc->SampleDesc.Count;
		desc.Usage = (D3D11_USAGE)Desc->Usage;
		desc.BindFlags = Desc->BindFlag;
		if (RawData) {
			Initial.pSysMem = RawData;
			Initial.SysMemPitch = Desc->Width;
			Initial.SysMemSlicePitch = 1;
			pInitial = &Initial;
		}
		Device->CreateTexture2D(&desc, pInitial, (ID3D11Texture2D**)&texture.Texture);
		D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
		srDesc.Format = desc.Format;
		if (desc.Format == FORMAT_R32_TYPELESS) {
			srDesc.Format = (DXGI_FORMAT)FORMAT_R32_FLOAT;
		}
		srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srDesc.Texture2D.MostDetailedMip = 0;
		srDesc.Texture2D.MipLevels = 1;
		Device->CreateShaderResourceView(texture.Texture, &srDesc, &texture.Resource);
		if (Desc->BindFlag & R_BIND_FLAG::BIND_RENDER_TARGET) {
			D3D11_RENDER_TARGET_VIEW_DESC rtDesc;
			rtDesc.Format = desc.Format;
			rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			rtDesc.Texture2D.MipSlice = 0;
			Device->CreateRenderTargetView(texture.Texture, &rtDesc, &texture.Target);
		}
		else if (Desc->BindFlag & R_BIND_FLAG::BIND_DEPTH_STENCIL) {
			D3D11_DEPTH_STENCIL_VIEW_DESC dsDesc;
			ZeroMemory(&dsDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
			dsDesc.Format = desc.Format;
			if (desc.Format == FORMAT_R32_TYPELESS) {
				dsDesc.Format = (DXGI_FORMAT)FORMAT_D32_FLOAT;
			}
			dsDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
			dsDesc.Texture2D.MipSlice = 0;
			Device->CreateDepthStencilView(texture.Texture, &dsDesc, &texture.Depth);
		}
	}
}

int D3D11Render::CreateTexture2D(R_TEXTURE2D_DESC* Desc, void * RawData, int Size, int DataFlag) {
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


int D3D11Render::CreateGeometry(void * VBuffer, unsigned int VBSize, unsigned int VertexSize, void * IBuffer, unsigned int INum, R_FORMAT IndexFormat) {
	printf("create Geometry VB %d IB %d\n", VBSize, INum);
	D3DGeometry Geometry = {};
	D3D11_BUFFER_DESC BufferDesc = {};
	D3D11_SUBRESOURCE_DATA Initial = {};
	BufferDesc.ByteWidth = VBSize;
	BufferDesc.MiscFlags = 0;
	BufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	BufferDesc.CPUAccessFlags = 0;

	Initial.pSysMem = VBuffer;
	Initial.SysMemPitch = VBSize;
	Initial.SysMemSlicePitch = 0;
	Device->CreateBuffer(&BufferDesc, &Initial, &Geometry.Vertex);

	//Create Index Buffer
	if (IBuffer) {
		BufferDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
		BufferDesc.ByteWidth = sizeof(WORD)* INum;
		Initial.pSysMem = IBuffer;
		Initial.SysMemPitch = sizeof(WORD) * INum;
		Initial.SysMemSlicePitch = 0;
		Device->CreateBuffer(&BufferDesc, &Initial, &Geometry.Index);
	}
	Geometry.VSize = VBSize;
	Geometry.INum = INum;
	Geometry.VBSize = VertexSize;
	int Id = Geometries.AddItem(Geometry);
	return Id;
}

int D3D11Render::CreateInputLayout(R_INPUT_ELEMENT * Element, int Count, void * ShaderCode, int Size) {
	D3DInoutLayout Layout = {};
	D3D11_INPUT_ELEMENT_DESC desc[32] = {};
	for (int i = 0; i < Count; i++) {
		D3D11_INPUT_ELEMENT_DESC * ds = &desc[i];
		ds->SemanticName = Element[i].Semantic;
		ds->SemanticIndex = Element[i].SemanticIndex;
		ds->Format = (DXGI_FORMAT)Element[i].Format;
		ds->InputSlot = Element[i].Slot;
		ds->InputSlotClass = (D3D11_INPUT_CLASSIFICATION)Element[i].Type;
		ds->AlignedByteOffset = Element[i].Offset;
	}
	Device->CreateInputLayout(desc, Count, ShaderCode, Size, &Layout.Layout);
	int Id = InputLayouts.AddItem(Layout);
	return Id;
}

int D3D11Render::CreateConstantBuffer(unsigned int Size) {
	printf("create Constentbuffer Size %d\n", Size);
	D3DConstant constant = {};
	D3D11_BUFFER_DESC BufferDesc;
	// Matrix
	ZeroMemory(&BufferDesc, sizeof(BufferDesc));
	BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	BufferDesc.ByteWidth = Size;
	BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	BufferDesc.CPUAccessFlags = 0;
	Device->CreateBuffer(&BufferDesc, NULL, &constant.Buffer);
	int Id = Constants.AddItem(constant);
	return Id;
}


int D3D11Render::CreateVertexShader(void * ByteCode, unsigned int Size, int flag) {
	printf("create VertexShader buffersize %d\n", Size);
	D3DRenderShader Shader = {};
	Device->CreateVertexShader(ByteCode, Size, NULL, &Shader.VS);
	int Id = Shaders.AddItem(Shader);
	return Id;
}

int D3D11Render::CreateGeometryShader(void * ByteCode, unsigned int Size, int flag) {
	return 2;
}

int D3D11Render::CreateHullShader(void * ByteCode, unsigned int Size, int flag) {
	return 3;
}

int D3D11Render::CreateDomainShader(void * ByteCode, unsigned int Size, int flag) {
	return 4;
}

int D3D11Render::CreatePixelShader(void * ByteCode, unsigned int Size, int flag) {
	printf("create PixelShader buffersize %d\n", Size);
	D3DRenderShader Shader = {};
	Device->CreatePixelShader(ByteCode, Size, NULL, &Shader.PS);
	int Id = Shaders.AddItem(Shader);
	return Id;
}

int D3D11Render::CreateComputeShader(void * ByteCode, unsigned int Size, int flag) {
	return 0;
}


int D3D11Render::CreateDepthStencilStatus(R_DEPTH_STENCIL_DESC* Desc) {
	D3D11_DEPTH_STENCIL_DESC d = {};
	d.DepthFunc = (D3D11_COMPARISON_FUNC)Desc->DepthFunc;
	d.DepthEnable = Desc->ZTestEnable;
	d.DepthWriteMask = (D3D11_DEPTH_WRITE_MASK)Desc->ZWriteEnable;
	d.StencilEnable = Desc->StencilEnable;
	d.StencilReadMask = 0xFF;
	d.StencilWriteMask = 0xFF;
	// front
	d.FrontFace.StencilFailOp = (D3D11_STENCIL_OP)Desc->StencilFailFront;
	d.FrontFace.StencilDepthFailOp = (D3D11_STENCIL_OP)Desc->DepthFailFront;
	d.FrontFace.StencilPassOp = (D3D11_STENCIL_OP)Desc->StencilPassFront;
	d.FrontFace.StencilFunc = (D3D11_COMPARISON_FUNC)Desc->StencilFuncFront;
	// back
	d.BackFace.StencilFailOp = (D3D11_STENCIL_OP)Desc->StencilFailBack;
	d.BackFace.StencilDepthFailOp = (D3D11_STENCIL_OP)Desc->DepthFailBack;
	d.BackFace.StencilPassOp = (D3D11_STENCIL_OP)Desc->StencilPassBack;
	d.BackFace.StencilFunc = (D3D11_COMPARISON_FUNC)Desc->StencilFuncBack;

	D3DRenderState State = {};
	State.StencilRef = Desc->StencilRef;
	Device->CreateDepthStencilState(&d, &State.Depth);

	int id = RenderState.AddItem(State);
	return id;
}

int D3D11Render::CreateBlendStatus(R_BLEND_STATUS* Desc) {
	D3D11_BLEND_DESC d = {};
	d.AlphaToCoverageEnable = 0;
	d.IndependentBlendEnable = 0;
	d.RenderTarget[0].BlendEnable = Desc->Enable;
	d.RenderTarget[0].BlendOp = (D3D11_BLEND_OP)(Desc->BlendOp);
	d.RenderTarget[0].BlendOpAlpha = (D3D11_BLEND_OP)(Desc->BlendOpAlpha);
	d.RenderTarget[0].SrcBlend = (D3D11_BLEND)(Desc->SrcBlend);
	d.RenderTarget[0].DestBlend = (D3D11_BLEND)(Desc->DestBlend);
	d.RenderTarget[0].SrcBlendAlpha = (D3D11_BLEND)(Desc->SrcBlendAlpha);
	d.RenderTarget[0].DestBlendAlpha = (D3D11_BLEND)(Desc->DestBlendAlpha);
	d.RenderTarget[0].RenderTargetWriteMask = Desc->Mask;
	D3DRenderState State = {};
	Device->CreateBlendState(&d, &State.Blend);
	int id = RenderState.AddItem(State);
	return id;
}

int D3D11Render::CreateRasterizerStatus(R_RASTERIZER_DESC* Desc) {
	D3D11_RASTERIZER_DESC d = {};
	d.FillMode = (D3D11_FILL_MODE)Desc->FillMode;
	d.CullMode = (D3D11_CULL_MODE)Desc->CullMode;
	// multisample
	d.MultisampleEnable = Desc->MultisampleEnable;
	d.AntialiasedLineEnable = Desc->AntialiasedLineEnable;
	d.FrontCounterClockwise = Desc->FrontCounterClockwise;
	D3DRenderState State = {};
	Device->CreateRasterizerState(&d, &State.Raster);
	int id = RenderState.AddItem(State);
	return id;
}


void D3D11Render::SetBlendStatus(int Blend) {
	if (Blend >= 0) {
		D3DRenderState& State = RenderState[Blend];
		DeviceContext->OMSetBlendState(State.Blend, 0, -1);
	}
}
// depth and stencil
void D3D11Render::SetDepthStencilStatus(int DepthStencil) {
	if (DepthStencil >= 0) {
		D3DRenderState& State = RenderState[DepthStencil];
		DeviceContext->OMSetDepthStencilState(State.Depth, State.StencilRef);
	}
}
// rasterizer
void D3D11Render::SetRasterizerStatus(int Rasterizer) {
	if (Rasterizer >= 0) {
		D3DRenderState& State = RenderState[Rasterizer];
		DeviceContext->RSSetState(State.Raster);
	}
}

// viewport
void D3D11Render::SetViewPort(float tlx, float tly, float width, float height, float minz, float maxz) {
	D3D11_VIEWPORT viewport;
	viewport.MinDepth = minz;
	viewport.MaxDepth = maxz;
	viewport.TopLeftX = tlx;
	viewport.TopLeftY = tly;
	viewport.Width = width;
	viewport.Height = height;
	DeviceContext->RSSetViewports(1, &viewport);
}


void D3D11Render::SetDepthStencil(int Depth) {
	if (Depth >= 0) {
		D3DTexture& Texture = Textures[Depth];
		this->Depth = Texture.Depth;
		DeviceContext->OMSetRenderTargets(0, 0, Texture.Depth);
	}
}

void D3D11Render::SetRenderTargets(int Count, int * Targets) {
	for (int i = 0; i < Count; i++) {
		D3DTexture& Texture = Textures[Targets[i]];
		this->Targets[i] = Texture.Target;
	}
	CurrentTargets = Count;
	DeviceContext->OMSetRenderTargets(Count, this->Targets, Depth);
}

void D3D11Render::SetTexture(int StartSlot, int * Texture, int Count) {
	ID3D11ShaderResourceView * Views[32];
	for (int i = 0; i < Count; i++) {
		D3DTexture& texture = Textures[Texture[i]];
		Views[i] = texture.Resource;
	}
	//printf("%d %d\n", StartSlot, Texture[0]);
	DeviceContext->VSSetShaderResources(StartSlot, Count, Views);
	DeviceContext->GSSetShaderResources(StartSlot, Count, Views);
	DeviceContext->PSSetShaderResources(StartSlot, Count, Views);
}

void D3D11Render::SetInputLayout(int Id) {
	if (Id >= 0) {
		D3DInputLayout& Layout = InputLayouts[Id];
		DeviceContext->IASetInputLayout(Layout.Layout);
	}
}

void D3D11Render::SetVertexShader(int Id) {
	if (Id >= 0) {
		D3DRenderShader& Shader = Shaders[Id];
		DeviceContext->VSSetShader(Shader.VS, 0, 0);
	}
}

void D3D11Render::SetPixelShader(int Id) {
	if (Id >= 0) {
		D3DRenderShader& Shader = Shaders[Id];
		DeviceContext->PSSetShader(Shader.PS, 0, 0);
	}
}

void D3D11Render::SetConstant(int Slot, int Buffer, void * CPUData, unsigned int Size) {
	if (Buffer >= 0) {
		D3DConstant& Constant = Constants[Buffer];
		DeviceContext->UpdateSubresource(Constant.Buffer, 0, 0, CPUData, 0, 0);
		DeviceContext->VSSetConstantBuffers(Slot, 1, &Constant.Buffer);
		DeviceContext->GSSetConstantBuffers(Slot, 1, &Constant.Buffer);
		DeviceContext->PSSetConstantBuffers(Slot, 1, &Constant.Buffer);
	}
}

void D3D11Render::ClearDepth(float depth, float stencil) {
	DeviceContext->ClearDepthStencilView(Depth, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
}

void D3D11Render::ClearRenderTarget(){
	XMFLOAT4 ClearColor(0.0f, 0.0f, 0.0f, 0);
	for (int i = 0; i < CurrentTargets; i++) {
		DeviceContext->ClearRenderTargetView(Targets[i], (float*)&ClearColor);
	}
}

void D3D11Render::Present() {
	SwapChain->Present(1, 0);
}

void D3D11Render::Draw(int Id) {
	if (Id >= 0) {
		D3DGeometry& Geometry = Geometries[Id];
		// just drar it
		DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		UINT Offset = 0;
		//Set IA
		DeviceContext->IASetVertexBuffers(0, 1, &Geometry.Vertex,  &Geometry.VBSize, &Offset);
		DeviceContext->IASetIndexBuffer(Geometry.Index, DXGI_FORMAT_R16_UINT, 0);
		DeviceContext->DrawIndexed(Geometry.INum, 0, 0);
	}
}

void D3D11Render::Quad() {
	// id zero is the quad geometry
	D3DGeometry& Geometry = Geometries[0];
		// just drar it
	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT Offset = 0;
	//Set IA
	DeviceContext->IASetVertexBuffers(0, 1, &Geometry.Vertex, &Geometry.VBSize, &Offset);
	DeviceContext->IASetIndexBuffer(Geometry.Index, DXGI_FORMAT_R16_UINT, 0);
	DeviceContext->DrawIndexed(Geometry.INum, 0, 0);
}