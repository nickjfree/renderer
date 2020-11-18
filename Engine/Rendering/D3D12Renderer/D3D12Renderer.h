#ifndef __D3D12_RENDERER__
#define __D3D12_RENDERER__

#include "windows.h"
#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_4.h>
#include "pix3.h"
#include "Rendering/RenderInterface.h"
#include "Rendering/RenderDesc.h"
#include "D3D12Helper.h"
#include "Tasks/Mutex.h"
#include "Container/List.h"
#include "Container/Vector.h"


namespace D3D12Renderer {

	constexpr auto max_descriptor_heap_size = 16384;
	/*
		command context
	*/
	class D3D12CommandContext : public Transient<D3D12CommandContext>
	{
		friend Transient<D3D12CommandContext>;
	public:
		// alloc transient command context
		static D3D12CommandContext* AllocTransient(ID3D12Device* d3d12Device, D3D12_COMMAND_LIST_TYPE cmdType);
		// alloc command context
		static D3D12CommandContext* Alloc(ID3D12Device* d3d12Device, D3D12_COMMAND_LIST_TYPE cmdType);
		// release
		void Release();
		// getcmdList
		ID3D12GraphicsCommandList1* GetCmdList() { return cmdList; }
		// flush
		UINT64 Flush(bool wait);
	private:
		// reset transient resource status
		void resetTransient();
		// create
		void create(ID3D12Device* d3d12Device, D3D12_COMMAND_LIST_TYPE cmdType);
	private:
		// context type
		enum class COMMAND_CONTEXT_TYPE {
			GRAPHIC,
			COMPUTE,
			COPY,
			COUNT,
		};
		// context type
		D3D12_COMMAND_LIST_TYPE  cmdType;
		// graphic cmdList
		ID3D12GraphicsCommandList1* cmdList = nullptr;
		// command allocator
		ID3D12CommandAllocator* cmdAllocator = nullptr;
		// rt commandlist
		ID3D12GraphicsCommandList5* rtCommandList = nullptr;
	};


	/*
		Descriptor Heap
	*/
	class D3D12DescriptorHeap : public Transient<D3D12DescriptorHeap>
	{
		friend Transient<D3D12DescriptorHeap>;
	public:
		// alloc
		static D3D12DescriptorHeap* Alloc(ID3D12Device* d3d12Device, unsigned int size, D3D12_DESCRIPTOR_HEAP_TYPE heapType, D3D12_DESCRIPTOR_HEAP_FLAGS heapFlag);
		// alloc transient
		static D3D12DescriptorHeap* AllocTransient(ID3D12Device* d3d12Device);
		// get gpu handle
		D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle(unsigned int index);
		// get cpu handle
		D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle(unsigned int index);
		// release
		void Release();
	private:
		// reset
		void resetTransient() { currentIndex = 0; }
		// Create
		void create(ID3D12Device* d3d12Device, unsigned int size, D3D12_DESCRIPTOR_HEAP_TYPE heapType, D3D12_DESCRIPTOR_HEAP_FLAGS heapFlag);

	public:
		// descripter type
		enum class DESCRIPTOR_HANDLE_TYPES {
			SRV,
			UAV,
			RTV,
			DSV,
			SAMPLER,
			COUNT,
		};
	private:
		// the heap
		ID3D12DescriptorHeap* descriptorHeap = nullptr;
		// size
		unsigned int size = 0;
		// int current index
		unsigned int currentIndex = 0;
		// increment size
		unsigned int incrementSize = 0;
	};


	/*
		root signature
	*/
	class D3D12RootSignature : public Transient<D3D12RootSignature>
	{
	public:

	private:
		// reset tansient resource
		void resetTransient() {};
	private:
		// rootsignature
		ID3D12RootSignature* rootSignature;
	};

	/*
		pipleline state
	*/
	class D3D12PipelineState
	{
	public:
	private:
		// pipeline state
		ID3D12PipelineState* pipelineState;
	};


	class D3D12Resource;

	/*
		the render interface
	*/
	class D3D12RenderInterface : public RenderInterface
	{
	public:
		int Initialize(int width, int height);
		// create texture
		int CreateTexture2D(R_TEXTURE2D_DESC* desc);
		// destory buffer
		int DestoryTexture2D(int id);
		// create texture
		int CreateBuffer(R_BUFFER_DESC* desc);
		// destory buffer
		int DestoryBuffer(int id);
	private:
		// get resource
		D3D12Resource* GetResource(int id);
		// destory resource
		void DestoryResource(int id);
		// init descriptor heaps
		void InitDescriptorHeaps();
		// init d3d12Device
		void InitD3D12Device();
	private:
		// device
		ID3D12Device* d3d12Device = nullptr;
		// rtxDevice
		ID3D12Device5* rtxDevice = nullptr;
		// descriptor heaps
		D3D12DescriptorHeap* descHeaps[(unsigned int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::COUNT];
		// back buffer size
		unsigned int backbufferWidth = 0;
		unsigned int backbufferHeight = 0;
	};


}

#endif