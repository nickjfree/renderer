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
#include "Container/HashMap.h"

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
	constexpr auto max_descriptor_table_size = 16;
	constexpr auto max_descriptor_table_num = 8;
	constexpr auto max_root_descriptor_num = 8;
	constexpr auto max_srv_slot_num = 64;
	constexpr auto max_uav_slot_num = 64;
	constexpr auto max_sampler_slot_num = 64;


	class D3D12RootSignature : public Transient<D3D12RootSignature>
	{
		friend Transient;
	public:
		// alloc transient
		static D3D12RootSignature* AllocTransient(ID3D12Device* d3d12Device, bool local);
		// set samplers
		void SetSamplerTable(ID3D12CommandList * cmdList, D3D12_GPU_DESCRIPTOR_HANDLE handle);
		// invalidate bindings
		void SetStale() { stale = true; }
	private:
		// create
		void create(ID3D12Device* d3d12Device, bool local);
		// init
		void initRootSignature(ID3D12Device* d3d12Device, bool local, D3D12_ROOT_PARAMETER1* rootParameters, int numRootParameters);
		// init desc table cache
		void initDescriptorTableCache(int tableIndex, int rootParameterIndex, D3D12_ROOT_PARAMETER1* rootParameter);
		// init mapping
		void initMapping(D3D12_ROOT_PARAMETER1* rootParameters, int numRootParameters);
		// reset tansient resource
		void resetTransient() {};
	private:
		// mappings
			// descriptor table cache
		typedef struct DescriptorTable {
			int rootSlot;
			int size;
			D3D12_DESCRIPTOR_RANGE_TYPE descriptorType;
			int resourceId[max_descriptor_table_size];
			D3D12_CPU_DESCRIPTOR_HANDLE handles[max_descriptor_table_size];
			bool dirty;
			bool invalid;
		} DescriptorTable;

		// descriptor table slot info
		typedef struct DescTableSlot {
			int rootSlot;
			int tableIndex;
			int offset;
		}DescTable;

		// constant buffer slot. Const buffers only use root parameters.
		typedef struct RootDescriptorSlot {
			int rootSlot;
			int dirty;
			union {
				D3D12_CONSTANT_BUFFER_VIEW_DESC constDesc;
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
			};
		} RootDescriptorSlot;
		// rootsignature
		ID3D12RootSignature* rootSignature;
		// bindings are stale, must be rebind
		bool stale = true;
		// local or not
		bool local = false;
		// descriptor tables
		DescriptorTable  descTables[max_descriptor_table_num];
		// root constants or srv
		RootDescriptorSlot rootDescriptors[max_root_descriptor_num];
		// srv slot
		DescTableSlot srvs[max_srv_slot_num];
		// uav slot
		DescTableSlot uavs[max_uav_slot_num];
		// sampler slot
		DescTableSlot samplers[max_sampler_slot_num];
		// desc table number
		int numDescriptorTables = 0;
		// root descriptor number
		int numRootDescriptors = 0;
	};

	/*
		pipleline state
	*/
	class D3D12PipelineStateCache
	{
	public:
		D3D12PipelineStateCache();
		// find pso by current psocache or create a new one
		static ID3D12PipelineState* GetPipelineState(ID3D12Device* d3d12Device, ID3D12RootSignature* rootSignature, const D3D12PipelineStateCache& cache);
		// operator int
		operator int() const;
		bool operator == (const D3D12PipelineStateCache& rh);
		bool operator != (const D3D12PipelineStateCache& rh);
	private:
		// create new pso
		static ID3D12PipelineState* CreatePipelineState(ID3D12Device* d3d12Device, ID3D12RootSignature* rootSignature, const D3D12PipelineStateCache& cache);
	private:
		// hash value
		mutable unsigned int hash;
	public:
		// dirty flag
		int Dirty;
		// ids of shaders
		int VS;
		int PS;
		int GS;
		int DS;
		int HS;
		// id of render state
		int Depth;
		int Rasterizer;
		int Blend;
		// int input element
		int InputLayout;
		// render target and format;
		int NumRTV;
		DXGI_FORMAT RTVFormat[8];
		DXGI_FORMAT DSVFormat;
		// geometry topology
		R_PRIMITIVE_TOPOLOGY_TYPE Top;
	private:
		// pipelinestate lookup table
		static HashMap<D3D12PipelineStateCache, ID3D12PipelineState*> psoTable;
		// lock
		static Mutex lock;
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
	class RingConstantBuffer;

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

		/*
		*  rendering functions
		*/
		

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
		// current rootsignatures
		D3D12RootSignature* graphicsRootSignature = nullptr;
		D3D12RootSignature* computeRootSignature = nullptr;
		// constant buffer allocater
		RingConstantBuffer* ringConstantBuffer = nullptr;
		// current pipeline state
		D3D12PipelineStateCache piplineStateCache;
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