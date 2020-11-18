#ifndef __D3D12_RESOURCE__
#define __D3D12_RESOURCE__

#include "D3D12Renderer.h"

namespace D3D12Renderer {

	// const buffer
	constexpr auto max_upload_heap_size = 2048 * 256;
	constexpr auto const_buffer_align = 256;

	// resource pool
	constexpr auto max_texture_number = 8192;
	constexpr auto max_buffer_number = 8192;
	constexpr auto max_geometry_number = 8192;

	/*
		resource describe
	*/
	typedef struct ResourceDescribe {
		union {
			R_TEXTURE2D_DESC textureDesc;
			R_BUFFER_DESC bufferDesc;
		};
	}ResourceDescribe;

	/*
		directx resource base class
	*/
	class D3D12Resource
	{
	public:		
		// Create
		virtual void Create(ID3D12Device * d3d12Device, ResourceDescribe* resourceDesc) = 0;
		// Release
		virtual void Release() = 0;
		// set state (issue a transfer barrier)
		void SetResourceState(D3D12CommandContext *cmdContext, D3D12_RESOURCE_STATES targetState);
		// srv
		D3D12_GPU_DESCRIPTOR_HANDLE  GetSrv();
		// uav
		D3D12_GPU_DESCRIPTOR_HANDLE  GetUav();
		// rtv
		D3D12_CPU_DESCRIPTOR_HANDLE  GetRtv();
		// dsv
		D3D12_CPU_DESCRIPTOR_HANDLE  GetDsv();
	public:
		// resource types
		enum class RESOURCE_TYPES {
			BUFFER,
			TEXTURE,
			GEOMETRY,
			BLAS,
			TLAS,
			COUNT,
		};
		// resource state
		D3D12_RESOURCE_STATES state;
	protected: 
		// views
		D3D12_CPU_DESCRIPTOR_HANDLE  views[(int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::COUNT];
		// resource
		ID3D12Resource* resource = nullptr;
	};

	/*
		pool resource	
	*/
	template <class T, int size>
	class PoolResource : public ResourcePool<T, size>, public D3D12Resource
	{
	public:
		static T* CreateResource(ID3D12Device* d3d12Device, ResourceDescribe* resourceDesc);
	};

	template <class T, int size> T* PoolResource<T, size>::CreateResource(ID3D12Device* d3d12Device, ResourceDescribe* resourceDesc)
	{
		T* resource = Alloc();
		resource->Create(d3d12Device, resourceDesc);
		return resource;
	}

	/*
		buffer (shader resource or uav)
	*/
	class BufferResource : public PoolResource<BufferResource, max_buffer_number>
	{
	public:
		// create
		void Create(ID3D12Device* d3d12Device, ResourceDescribe* resourceDesc);
		// release
		void Release();
	};

	/*
		texture ( render targets, shader resource or uav)
	*/
	class TextureResource : public PoolResource<BufferResource, max_texture_number>
	{
	public:
		// create
		void Create(ID3D12Device* d3d12Device, ResourceDescribe* resourceDesc);
		// release
		void Release();
	private:
		// upload 
		void upload(ID3D12Device* d3d12Device, void* cpuData, unsigned int size);
	private:
		// isCube
		bool isCube = false;
	};

	/*
		vertexbuffer + indexbuffer
	*/
	class Geometry: public PoolResource<BufferResource, max_geometry_number>
	{
	public:
	public:
		// create
		void Create(ID3D12Device* d3d12Device, ResourceDescribe* resourceDesc);
		// release
		void Release();
	private:
		BufferResource* vertextBuffer;
		BufferResource* indexBuffer;
	};


	/*
		UploadHeap
	*/
	class UploadHeap : public Transient<UploadHeap>
	{
		friend Transient<UploadHeap>;
	public:
		// Alloc transient
		static UploadHeap* AllocTransient(ID3D12Device* d3d12Device, unsigned int size);
		// Alloc
		static UploadHeap* Alloc(ID3D12Device* d3d12Device, unsigned int size);
		// release resource
		void Release();
		// suballoc
		bool SubAlloc(unsigned int allocSize);
		// gpu virtual address
		D3D12_GPU_VIRTUAL_ADDRESS GetCurrentGpuVirtualAddress() { return resource->GetGPUVirtualAddress() + currentOffset; }
		// cpu address
		void*  GetCurrentCpuVirtualAddress() { return (void*)((UINT64)cpuBaseAddress + currentOffset); }
		// get
		ID3D12Resource* Get() { return resource; }
	private:
		// reset
		void resetTransient() { currentOffset = 0; }
		// create 
		void create(ID3D12Device* d3d12Device, unsigned int size);
	private:
		// resource
		ID3D12Resource* resource;
		// current offset
		unsigned int currentOffset = 0;
		// ALIGN
		unsigned int align = 256;
		// size
		unsigned int size;
		// mapped cpu address
		void* cpuBaseAddress = nullptr;
	};

	/*
		ring constant buffer
	*/
	class RingConstantBuffer
	{
	public:
		// Alloc transient constant buffer
		void* AllocTransientConstantBuffer(unsigned int size, void ** gpuAddress);
		// reset
		void Reset();
	private:
		// current upload heap
		UploadHeap* currentUploadHeap = nullptr;
		// device
		ID3D12Device* d3d12Device;
	};
}


#endif

