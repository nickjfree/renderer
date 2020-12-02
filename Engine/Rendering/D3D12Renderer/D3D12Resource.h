#ifndef __D3D12_RESOURCE__
#define __D3D12_RESOURCE__

#include "D3D12Common.h"

namespace D3D12Renderer {

	class UploadHeap;

	// const buffer
	constexpr auto max_upload_heap_size = 2048 * 256;
	constexpr auto const_buffer_align = 256;

	// resource pool
	constexpr auto max_texture_number = 8192;
	constexpr auto max_buffer_number = 8192;
	constexpr auto max_geometry_number = 8192;

	// backbuffer count
	constexpr auto backbuffer_count = 2;

	/*
		resource describe
	*/
	typedef struct ResourceDescribe {
		union {
			R_TEXTURE2D_DESC textureDesc;
			R_BUFFER_DESC bufferDesc;
			R_GEOMETRY_DESC geometryDesc;
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
		// Get 
		ID3D12Resource* GetResource() { return resource; }
		// set state (issue a transfer barrier)
		void SetResourceState(D3D12CommandContext *cmdContext, D3D12_RESOURCE_STATES targetState);
		// create descriptor handles in cpuHeap
		virtual void CreateViews(ID3D12Device* d3d12Device, ResourceDescribe* resourceDesc, D3D12DescriptorHeap** descHeaps);
		// srv
		D3D12_CPU_DESCRIPTOR_HANDLE  GetSrv() 
		{ 
			return views[static_cast<int>(D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::SRV)];
		}
		// uav
		D3D12_CPU_DESCRIPTOR_HANDLE  GetUav() 
		{
			return views[static_cast<int>(D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::UAV)];
		}
		// rtv
		D3D12_CPU_DESCRIPTOR_HANDLE  GetRtv()
		{
			return views[static_cast<int>(D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::RTV)];
		}
		// dsv
		D3D12_CPU_DESCRIPTOR_HANDLE  GetDsv()
		{
			return views[static_cast<int>(D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::DSV)];
		}
	private:

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
		D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
	protected: 
		// views
		D3D12_CPU_DESCRIPTOR_HANDLE  views[(int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::COUNT] = {};
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
		// update
		void Upload(ID3D12Device* d3d12Device, D3D12CommandContext* copyContext, UploadHeap* uploadHeap, void* cpuData, unsigned int size);
		// create descriptors in cpu heap
		void CreateViews(ID3D12Device* d3d12Device, ResourceDescribe* resourceDesc, D3D12DescriptorHeap** descHeaps);
	};


	/*
		texture ( render targets, shader resource or uav)
	*/
	class D3D12BackBuffer;

	class TextureResource : public PoolResource<BufferResource, max_texture_number>
	{
		friend D3D12BackBuffer;
	public:
		// create
		void Create(ID3D12Device* d3d12Device, ResourceDescribe* resourceDesc);
		// release
		void Release();
		// upload 
		void Upload(ID3D12Device* d3d12Device, D3D12CommandContext* cmdContext, UploadHeap* uploadHeap, std::vector<D3D12_SUBRESOURCE_DATA>& subresources);
		// create descriptors in cpu heap
		void CreateViews(ID3D12Device* d3d12Device, ResourceDescribe* resourceDesc, D3D12DescriptorHeap** descHeaps);
	private:
		// isCube
		bool isCube = false;
		// rtv dsv format used for pipelinestate
		union {
			DXGI_FORMAT dsvFormat;
			DXGI_FORMAT rtvFormat;
		};
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
		// buffers
		BufferResource* vertexBuffer;
		BufferResource* indexBuffer;
		// vertex stride
		unsigned int vertexStride;
		// vertex size
		unsigned int vertexBufferSize;
		// index num
		unsigned int numIndices;
		// toplogy format
		R_PRIMITIVE_TOPOLOGY primitiveToplogy;
	};

	/*
		backbuffer
	*/
	class D3D12BackBuffer
	{
	public:
		// create
		void Create(ID3D12Device* d3d12Device, IDXGIFactory4* pFactory, HWND hWnd, int width, int height, D3D12DescriptorHeap* rtvHeap);
		// get rtv
		D3D12_CPU_DESCRIPTOR_HANDLE GetRtv();
		// GetResource
		ID3D12Resource* GetResource();
		// get frameIndex
		int GetFrameIndex() { return frameIndex; }
		// present
		UINT64 Present(D3D12CommandContext* cmdContext);
		// set status
		void SetResourceState(D3D12CommandContext* cmdContext, D3D12_RESOURCE_STATES targetState);
		// wait for next
		void WaitForNextFrame();
	public:
		// backbuffer size
		int width;
		int height;
	private:
		// backbufer textures
		TextureResource backBuffers[backbuffer_count];
		// swapchains
		IDXGISwapChain3* swapChain = nullptr;
		// current backbuffer index
		int frameIndex = -1;
		// prev frame fence
		UINT64 prevFrameFence[backbuffer_count];
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
		static UploadHeap* Alloc(ID3D12Device* d3d12Device, UINT64 size);
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
		void create(ID3D12Device* d3d12Device, UINT64 size);
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
		// release
		void Release();
		// create
		static RingConstantBuffer* Alloc(ID3D12Device* d3d12Device);
	private:
		// current upload heap
		UploadHeap* currentUploadHeap = nullptr;
		// device
		ID3D12Device* d3d12Device = nullptr;
	};

	/*
		input layouts	
	*/
	class D3D12InputLayout : public ResourcePool<D3D12InputLayout, 32>
	{
	public:
		D3D12_INPUT_LAYOUT_DESC Layout;
		D3D12_INPUT_ELEMENT_DESC Element[32];
		char Names[32][32];

	};

	/* 
		shaders 
	*/
	class D3D12Shader : public ResourcePool<D3D12Shader, 32>
	{
	public:
		D3D12_SHADER_BYTECODE ByteCode;
		void* RawCode;
	};

	/*
		state describe
	*/
	typedef struct StateDescribe {
		union {
			R_BLEND_STATUS blendDesc;
			R_RASTERIZER_DESC rasterizerDesc;
			R_DEPTH_STENCIL_DESC depthStencilDesc;
		};
	}StateDescribe;

	/*
	*	render state
	*/
	class D3D12RenderState : public ResourcePool<D3D12RenderState, 128> 
	{
	public:
		union {
			D3D12_DEPTH_STENCIL_DESC Depth;
			D3D12_RASTERIZER_DESC Raster;
			D3D12_BLEND_DESC Blend;
		};
		unsigned char StencilRef;
	};

}


#endif

