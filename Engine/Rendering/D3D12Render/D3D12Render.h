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
#include "Container\HashMap.h"
#include "Structs.h"
#include "CommandQueue.h"
#include "CommandContext.h"
#include "Heap.h"
#include "DescriptorHeap.h"
#include "PSOCache.h"
#include "RootSignature.h"
#include "RaytracingScene.h"


namespace D3D12API {

#define MAX_TEXTURE_SIZE     8192
#define MAX_GEOMETRY_SIZE    4096
#define MAX_RESOURCE_BARRIER 2048

	typedef struct PerformaceInfo {
		LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
		LARGE_INTEGER Frequency;
		unsigned int DrawCallCount;
	}PerformaceInfo;



	class D3D12Render : public RenderInterface
	{
	private:
		static D3D12Render* thisRender;
		HWND hWnd;
		static const UINT FrameCount = NUM_FRAMES;
		// D3D12 device and somte other interfaces
		ID3D12Device* Device;
		// rtx device
		ID3D12Device5* rtxDevice;
		IDXGIAdapter* pAdapter;
		IDXGIAdapter3* pAdapter3;
		IDXGISwapChain3* SwapChain;
		D3D_FEATURE_LEVEL  FeatureLevel;
		ID3D12Resource* RenderTargets[FrameCount];
		ID3D12DescriptorHeap* RtvHeap;
		ID3D12PipelineState* PipelineState;
		UINT RtvDescriptorSize;
		// Synchronization objects.
		UINT FrameIndex;
		// command queues
		CommandQueue* CommandQueues[4];
		// main render target width and height
		int Width;
		int Height;
		// managers 
		LinearBuffer<D3DBuffer, 1024> Buffers;
		LinearBuffer<D3DTexture, 8192> Textures;
		LinearBuffer<D3DGeometry, 4096> Geometries;
		LinearBuffer<D3DInputLayout, 128> InputLayouts;
		LinearBuffer<D3DRenderShader, 512> Shaders;
		LinearBuffer<D3DConstant, 128> Constants;
		LinearBuffer<D3DRenderState, 128> RenderState;
		LinearBuffer<D3DBottomLevelAS, 8192> BottomLevelAS;
		// constant buffer heaps
		Vector<Heap*> UsedConstHeaps;
		// current heaps
		Heap* CurrentConstHeap;
		// resource descriptor heaps
		Vector<DescriptorHeap*> CpuSRVHeaps[NUM_FRAMES];
		// TRV Heaps
		Vector<DescriptorHeap*> CpuRTVHeaps[NUM_FRAMES];
		// DSV Heaps
		Vector<DescriptorHeap*> CpuDSVHeaps[NUM_FRAMES];
		// UAV Heaps for texture
		Vector<DescriptorHeap*> CpuTextureUAVHeaps[NUM_FRAMES];
		// UAV Heaps for buffer
		Vector<DescriptorHeap*> CpuBufferUAVHeaps[NUM_FRAMES];
		// UAV Heaps for deformed blas
		Vector<DescriptorHeap*> CpuBLASUAVHeaps[NUM_FRAMES];
		// SRV Heaps for buffer
		Vector<DescriptorHeap*> CpuBufferSRVHeaps[NUM_FRAMES];
		// SRV Heaps for deformaed blas
		Vector<DescriptorHeap*> CpuBLASSRVHeaps[NUM_FRAMES];
		// Sampler Heaps
		Vector<DescriptorHeap*> GpuSamplerHeaps;
		// raytracing scene
		RaytracingScene * rtScene[NUM_FRAMES];
		// null descriptor Heaps
		DescriptorHeap* NullHeap;
		// Used SRVHeaps
		Vector<DescriptorHeap*> UsedGpuSRVHeaps;
		// current SRV heap
		DescriptorHeap* CurrentSRVHeap;
		// PSO Table
		HashMap<PSOCache, ID3D12PipelineState*> PSOTable;
		// Root Signature
		RootSignature* RootSig;
		// current PSO
		PSOCache CurrentPSO;
		// resource barriar list
		Vector<CD3DX12_RESOURCE_BARRIER> ResourceBarriers;
		// current command context
		CommandContext* CurrentCommandContext;
		// current targets.
		D3D12_CPU_DESCRIPTOR_HANDLE Targets[8];
		int NumTargets;
		int CurrentTargets[8];
		// current depth
		D3D12_CPU_DESCRIPTOR_HANDLE Depth;
		// targets dirt
		int TargetDirty;
		// quad geometry id
		int QuadId;
		// null textureid
		int NullId;
		// null uav
		int NullUAV;
		// viewport size
		int ViewPortWidth;
		int ViewPortHeight;
		// previous frame's fence value
		UINT64 PrevFenceValue[NUM_FRAMES];
		// barrier flushed
		bool BarrierFlushed;
		// windows title
		char WindowTitle[1024];
		// perfomence counters
		PerformaceInfo Performance;
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
		// init decriptor heaps
		void InitDescriptorHeaps();
		// init Root Signature
		void InitRootSignature();
		// init samplers
		void InitSamplers();
		// init null texture
		void InitNullTexture();
		// init null uav
		void InitNullUAV();
		// create texture dds
		void CreateTextureDDS(D3DTexture& Texture, void* ddsData, int Size, bool* isCube);
		// create raw textures
		void CreateTexture2DRaw(R_TEXTURE2D_DESC* Desc, D3DTexture& Texture, void* ddsData, int Size);
		// helpers
		void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter);
		// wait
		void WaitForPreviousFrame();
		// create pso
		ID3D12PipelineState* CreatePSO(PSOCache& cache);
		// nswap command context
		void SwapCommandContext();
		// apply pso
		void FlushPSO();
		// flush barriers
		int FlushResourceBarriers();
		// flush rootsig
		void FlushRootSignature();
		// flush rendertargets
		void FlushRenderTargets();
		// build raytracing scene
		void BuildRaytracingScene();
		// wait raytracing scene
		void WaitRaytracingScene();
		// update performance info
		void ShowPerformanceInfo();
		// get topology_type by topology
		R_PRIMITIVE_TOPOLOGY_TYPE GetPtimitiveTopologyType(R_PRIMITIVE_TOPOLOGY topology);
	public:
		static D3D12Render* GetRender() { return thisRender; }
		// get queue
		CommandQueue* GetQueue(D3D12_COMMAND_LIST_TYPE type) { return CommandQueues[type]; };
	public:
		virtual int Initialize(int Width, int Height);

		// create buffer
		virtual int  CreateBuffer(R_BUFFER_DESC* desc);

		// create texture
		virtual int  CreateTexture2D(R_TEXTURE2D_DESC* Desc, void* RawData, int Size, int DataFlag);
		// create geometry. with raw vertext and index datas. the buffer pool is set to dynamic by default
		virtual int CreateGeometry(void* VBuffer, unsigned int VBSize, unsigned int VertexSize, void* IBuffer, unsigned int IBSize,
			R_FORMAT IndexFormat, R_PRIMITIVE_TOPOLOGY Top);
		// create bottom level as
		virtual int CreateRaytracingGeometry(int GeometryId, bool Deformable, int* BufferId);
		// add raytracing instance
		virtual int AddRaytracingInstance(R_RAYTRACING_INSTANCE& instance);

		virtual int CreateInputLayout(R_INPUT_ELEMENT* Element, int Count, void* ShaderCode, int Size);
		virtual int CreateVertexShader(void* ByteCode, unsigned int Size, int flag);
		virtual int CreateGeometryShader(void* ByteCode, unsigned int Size, int flag);
		virtual int CreateHullShader(void* ByteCode, unsigned int Size, int flag);
		virtual int CreateDomainShader(void* ByteCode, unsigned int Size, int flag);
		virtual int CreatePixelShader(void* ByteCode, unsigned int Size, int flag);
		virtual int CreateComputeShader(void* ByteCode, unsigned int Size, int flag);
		virtual int CreateConstantBuffer(unsigned int Size);

		virtual int CreateDepthStencilStatus(R_DEPTH_STENCIL_DESC* Desc);
		// Blend Status
		virtual int CreateBlendStatus(R_BLEND_STATUS* Desc);
		//Rasterizer status
		virtual int CreateRasterizerStatus(R_RASTERIZER_DESC* Desc);


		// resource destrcutors

		// destroy geometry data. free gpu resource
		virtual int DestroyGeometry(int Id);
		// destroy buffer

		virtual int DestroyBuffer(int Id);
		// destroy texture

		virtual int DestroyTexture2D(int Id);

		// set 
		virtual void SetRenderTargets(int Count, int* Targets);

		virtual void SetDepthStencil(int Depth);

		// set texture as uav
		virtual void SetUnorderedAccessTexture(int StartSlot, int* Textures, int Count);
		// set buffer as uav
		virtual void SetUnorderedAccessBuffer(int StartSlot, int* Buffers, int Count);
		// set texture as srv
		virtual void SetTexture(int StartSlot, int* Texture, int Count);
		// set buffer as srv
		virtual void SetBuffer(int StartSlot, int* Buffers, int Count);
		// set input layout
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
		virtual void SetConstant(int Slot, int Buffer, void* CPUData, unsigned int Size);

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
		virtual void DrawInstance(int Geometry, void* InstanceBuffer, unsigned int BufferSize, unsigned int InstanceNum);
		// Draw raw data. Slow operation, cause the data must copy to GPU memory first
		virtual void DrawRaw(void* VBuffer, unsigned int VBSize, unsigned int VertexSize, void* IBuffer, unsigned int IBSize, R_FORMAT IndexFormat) {}
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
