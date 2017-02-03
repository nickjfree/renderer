#ifndef __D3D12_RENDER__
#define __D3D12_RENDER__

/*
	Direct3D11 Renderinterface
*/
#include "Rendering\Renderinterface.h"
#include "windows.h"
#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_4.h>
#include <Objbase.h>
#include "Container\LinearBuffer.h"
#include "Structs.h"
#include "CommandQueue.h"
#include "CommandContext.h"
#include "Heap.h"



namespace D3D12API {
	


	class D3D12Render : public RenderInterface
	{
	private:
		static D3D12Render * thisRender;
		HWND hWnd;

		static const UINT FrameCount = NUM_FRAMES;
		// D3D12 device and somte other interfaces
		ID3D12Device *  Device;
		IDXGIAdapter * pAdapter;
		IDXGIAdapter3 * pAdapter3;
		IDXGISwapChain3 * SwapChain;
		D3D_FEATURE_LEVEL  FeatureLevel;
		ID3D12Resource * RenderTargets[FrameCount];
		ID3D12CommandAllocator * CommandAllocator;
		ID3D12CommandQueue * CommandQueueTest;
		ID3D12DescriptorHeap * RtvHeap;
		ID3D12PipelineState * PipelineState;
		ID3D12GraphicsCommandList * CommandList;
		UINT RtvDescriptorSize;

		// Synchronization objects.
		UINT FrameIndex;
		HANDLE FenceEvent;
		ID3D12Fence * Fence;
		UINT64 FenceValue;
		// command queues
		CommandQueue * CommandQueues[4];


		// main render target width and height
		int Width;
		int Height;
		// managers 
		LinearBuffer<D3DTexture, 8192> Textures;
		LinearBuffer<D3DGeometry, 4096> Geometries;
		LinearBuffer<D3DInputLayout, 128> InputLayouts;
		LinearBuffer<D3DRenderShader, 512> Shaders;
		LinearBuffer<D3DConstant, 128> Constants;
		LinearBuffer<D3DRenderState, 128> RenderState;
		// constant buffer heaps
		Vector<Heap*> UsedConstHeaps;
		// current heaps
		Heap * CurrentConstHeap;
		// current status
//		ID3D12RenderTargetView * Targets[8];
		int CurrentTargets;
//		ID3D12DepthStencilView * Depth;
	public:
		D3D12Render();
		~D3D12Render();
	private:
		// create windows
		HWND CreateRenderWindow(void);
		// create D3D12
		void InitD3D12();
		// init short operations
		void InitShortOperation();
		// create queues
		void InitQueues();

		// create texture dds
		void CreateTextureDDS(D3DTexture& Texture, void * ddsData, int Size);

		void CreateTexture2DRaw(R_TEXTURE2D_DESC* Desc, D3DTexture& Texture, void * ddsData, int Size);
		// helpers
		void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter);
		// wait
		void WaitForPreviousFrame();
	public:
		static D3D12Render * GetRender() { return thisRender; }
		// get queue
		CommandQueue * GetQueue(D3D12_COMMAND_LIST_TYPE type) { return CommandQueues[type]; };
	public:
		virtual int Initialize(int Widthm, int Height);

		virtual int  CreateTexture2D(R_TEXTURE2D_DESC* Desc, void * RawData, int Size, int DataFlag);
		// create geometry. with raw vertext and index datas. the buffer pool is set to dynamic by default
		virtual int CreateGeometry(void * VBuffer, unsigned int VBSize, unsigned int VertexSize, void * IBuffer, unsigned int IBSize, R_FORMAT IndexFormat);
		virtual int CreateInputLayout(R_INPUT_ELEMENT * Element, int Count, void * ShaderCode, int Size);
		virtual int CreateVertexShader(void * ByteCode, unsigned int Size, int flag);
		virtual int CreateGeometryShader(void * ByteCode, unsigned int Size, int flag);
		virtual int CreateHullShader(void * ByteCode, unsigned int Size, int flag);
		virtual int CreateDomainShader(void * ByteCode, unsigned int Size, int flag);
		virtual int CreatePixelShader(void * ByteCode, unsigned int Size, int flag);
		virtual int CreateComputeShader(void * ByteCode, unsigned int Size, int flag);
		virtual int CreateConstantBuffer(unsigned int Size);

		virtual int CreateDepthStencilStatus(R_DEPTH_STENCIL_DESC* Desc);
		// Blend Status
		virtual int CreateBlendStatus(R_BLEND_STATUS* Desc);
		//Rasterizer status
		virtual int CreateRasterizerStatus(R_RASTERIZER_DESC* Desc);

		// set 
		virtual void SetRenderTargets(int Count, int * Targets);

		virtual void SetDepthStencil(int Depth);

		virtual void SetTexture(int StartSlot, int * Texture, int Count);

		virtual void SetInputLayout(int Id);
		// set vertex shader
		virtual void SetVertexShader(int Shader);
		// set geometry shader
		virtual void SetGeometryShader(int Shader) {}
		// set hull shader
		virtual void SetHullShader(int Shader) {}
		// set domain shader
		virtual void SetDomainShader(int Shader) {}
		// set pixel shader
		virtual void SetPixelShader(int Shader);
		// set compute shader
		virtual void SetComputeShader(int Shader) {}
		// constant buffer
		virtual void SetConstant(int Slot, int Buffer, void * CPUData, unsigned int Size);

		// render state
		virtual void SetBlendStatus(int Blend);
		// depth and stencil
		virtual void SetDepthStencilStatus(int DepthStencil);
		// rasterizer
		virtual void SetRasterizerStatus(int Rasterizer);
		// viewport
		virtual void SetViewPort(float tlx, float tly, float width, float height, float minz, float maxz);
		// draw single geometry
		virtual void Draw(int Geometry);
		// draw instance geometry
		virtual void DrawInstance(int Geometry, void * InstanceBuffer, unsigned int BufferSize, unsigned int InstanceNum) {};
		// Draw raw data. Slow operation, cause the data must copy to GPU memory first
		virtual void DrawRaw(void * VBuffer, unsigned int VBSize, unsigned int VertexSize, void * IBuffer, unsigned int IBSize, R_FORMAT IndexFormat) {}
		// draw full screen quad
		virtual void Quad();
		// draw rect quad, not that usefull
		virtual void Rect() {};
		// clear depth
		virtual void ClearDepth(float depth, float stencil);
		// clear target
		virtual void ClearRenderTarget();
		// present
	    virtual void Present();
	};

}
#endif
