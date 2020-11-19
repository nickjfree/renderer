#ifndef __D3D12_COMMON__
#define __D3D12_COMMON__

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
	/*
	   Descriptor Heap
	*/
	constexpr auto max_descriptor_heap_size = 16384;

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


	// context type
	enum class COMMAND_CONTEXT_TYPE {
		GRAPHIC,
		BUNDLE,
		COMPUTE,
		COPY,
		COUNT,
	};

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
		// add barrier
		void AddBarrier(D3D12_RESOURCE_BARRIER& barrier);
	private:
		// reset transient resource status
		void resetTransient();
		// create
		void create(ID3D12Device* d3d12Device, D3D12_COMMAND_LIST_TYPE cmdType);
		// apply barriers
		void applyBarriers();
	private:
		// context type
		D3D12_COMMAND_LIST_TYPE  cmdType = D3D12_COMMAND_LIST_TYPE_DIRECT;
		// graphic cmdList
		ID3D12GraphicsCommandList1* cmdList = nullptr;
		// command allocator
		ID3D12CommandAllocator* cmdAllocator = nullptr;
		// rt commandlist
		ID3D12GraphicsCommandList5* rtCommandList = nullptr;
		// resource barriers
		Vector<D3D12_RESOURCE_BARRIER> barriers;
	};


	/*
		CommandQueue
	*/
	class D3D12CommandQueue
	{
	public:
		// alloc command queue
		static D3D12CommandQueue* Alloc(ID3D12Device* d3d12Device, D3D12_COMMAND_LIST_TYPE cmdType);
		// get cmdQueue by type
		static D3D12CommandQueue* GetQueue(D3D12_COMMAND_LIST_TYPE cmdType) { return queues[cmdType]; }
		// get cmdQueue resource
		ID3D12CommandQueue* Get() { return cmdQueue; }
		// get fence
		ID3D12Fence* GetFence() { return cmdFence; }
		// isFenceComplete
		bool IsFenceComplete(UINT64 fence);
		// wait for fence in gpu
		void GpuWait(ID3D12Fence* d3d12Fence, UINT64 fenceValue);
		// wait for fence in cpu
		void CpuWait(UINT64 fenceValue);
		// execute commandcontext
		UINT64 ExecuteCommandList(ID3D12CommandList* cmdList);
	private:
		// command queue resource
		ID3D12CommandQueue* cmdQueue = nullptr;
		// fence
		ID3D12Fence* cmdFence = nullptr;
		// current fenceValue
		UINT64  currentFenceValue = 0;
		// event handle
		HANDLE hEvent = 0;
		// mutext
		Mutex lock;
		// static queue
		static D3D12CommandQueue* queues[(unsigned int)COMMAND_CONTEXT_TYPE::COUNT];
	};

}

#endif