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
	constexpr auto max_rt_geometry_number = 8192;

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
			R_RT_GEOMETRY_DESC rtGeometryDesc;
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
		virtual void SetResourceState(D3D12CommandContext *cmdContext, D3D12_RESOURCE_STATES targetState);
		// get state
		virtual D3D12_RESOURCE_STATES GetResourceState() { return state; }
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

	class TextureResource : public PoolResource<TextureResource, max_texture_number>
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
			DXGI_FORMAT dsvFormat = DXGI_FORMAT_UNKNOWN;
			DXGI_FORMAT rtvFormat;
		};
	};

	/*
		raytracing as
	*/
	class Geometry;

	class RaytracingGeomtry: public PoolResource<RaytracingGeomtry, max_rt_geometry_number>
	{
	public:
		// retire all
		static void RetireAllTransientGeometry();
		// create
		void Create(ID3D12Device* d3d12Device, ResourceDescribe* resourceDesc);
		// retire
		void Retire();
		// release
		void Release();
		// transinet
		void SetTransient();
		// set resource state
		virtual void SetResourceState(D3D12CommandContext* cmdContext, D3D12_RESOURCE_STATES targetState);
		// isInitialized
		bool IsInitialized() { return initialized; }
		// pre-build
		void PreBuild(D3D12CommandContext* cmdContext);
		// build
		void Build(D3D12CommandContext* cmdContext);
		// post-build
		void PostBuild(D3D12CommandContext* cmdContext);
		// GetBottomLevel
		ID3D12Resource* GetBottomLevel() { return asBuffer; }
	private:
		// parent geometry
		Geometry* geometry = nullptr;
		// transient
		bool isTransient = false;
		// init
		bool initialized = false;
		// as buffers
		BufferResource* transientBuffer = nullptr;
		ID3D12Resource* asBuffer = nullptr;
		ID3D12Resource* scratchBuffer = nullptr;
		// inflight transient items
		static Vector<RaytracingGeomtry*> inflightRtGeometries;
		// geometry desc
		D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
		// build inputs
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelInputs = {};
	};

	/*
		vertexbuffer + indexbuffer
	*/
	class Geometry: public PoolResource<Geometry, max_geometry_number>
	{
		friend D3D12CommandContext;
		friend RaytracingGeomtry;
	public:
		// create
		void Create(ID3D12Device* d3d12Device, ResourceDescribe* resourceDesc);
		// release
		void Release();
		// create regeomtry
		int CreateRtGeometry(ID3D12Device* d3d12Device, bool isTransient);
	private:
		// buffers
		BufferResource* vertexBuffer = nullptr;
		BufferResource* indexBuffer = nullptr;
		// vertex stride
		unsigned int vertexStride = 0;
		// vertex size
		unsigned int vertexBufferSize = 0;
		// index num
		unsigned int numIndices = 0;
		// toplogy format
		R_PRIMITIVE_TOPOLOGY primitiveToplogy = R_PRIMITIVE_TOPOLOGY_UNDEFINED;
		// rt geometries
		Vector<RaytracingGeomtry*> transientRtGeometries;
		// static rtgeometry
		RaytracingGeomtry* staticRtGeometry = nullptr;
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
		HWND hWnd;
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
		D3D12_GPU_VIRTUAL_ADDRESS GetCurrentGpuVirtualAddress();
		// cpu address
		void* GetCurrentCpuVirtualAddress();
		// get
		ID3D12Resource* Get() { return resource; }
	private:
		// reset
		void resetTransient() { currentOffset = 0; currentRear = 0; }
		// create 
		void create(ID3D12Device* d3d12Device, UINT64 size);
	private:
		// resource
		ID3D12Resource* resource = nullptr;
		// current offset
		unsigned int currentOffset = 0;
		// current end offset
		unsigned int currentRear = 0;
		// ALIGN
		unsigned int align = 256;
		// size
		UINT64 size = 0;
		// mapped cpu address
		void* cpuBaseAddress = nullptr;
	};


	/*
		shader record
	*/
	typedef struct ShaderIdetifier {
		unsigned char identifier[D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES];
	}ShaderIdetifier;


	typedef struct ShaderRecord
	{
		// shader identifier
		ShaderIdetifier identifier[D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES];
		// 4 root parameters 32 bytes
		UINT64 rootParams[4];
	}ShaderRecord;

	/*
		shader binding table
	*/
	constexpr auto max_ray_types = 4;
	constexpr auto default_sbt_size = 1024 * 1024;

	class ShaderBindingTable
	{
	public:
		// alloc
		ShaderRecord* AllocShaderRecord(int materialId);
		// reset
		void Reset();
		// create
		void Create(ID3D12Device* d3d12Device);
	private:
		// SBT data
		Vector<ShaderRecord> hitGroups;
		// raygen table
		ShaderRecord rayGen[max_ray_types];
		// miss table
		ShaderRecord miss[max_ray_types];
		// sbt size
		UINT64 sbtSize = 0;
		// sbt in gpu
		ID3D12Resource* sbt = nullptr;
		// upload heap
		ID3D12Resource* sbtCpu = nullptr;
		// sbt ptr
		void* sbtPtr = nullptr;
		// devuce
		ID3D12Device* d3d12Device = nullptr;
	};

	/*
		rt Scene
	*/
	constexpr auto default_top_level_as_size = 1024 * 1024 * 4;

	class RaytracingScene : public Transient<RaytracingScene>
	{
		friend Transient<RaytracingScene>;
	public:
		// Alloc transient
		static RaytracingScene* AllocTransient(ID3D12Device* d3d12Device);
		// add instance
		void AddInstance(RaytracingGeomtry* rtGeometry, Matrix4x4& transform);
		// build
		void Build(D3D12CommandContext* cmdContext);
		// trace ray
		void TraceRay(D3D12CommandContext* cmdContext, int shaderIndex, unsigned int width, unsigned int height);
		// get desc heap
		D3D12DescriptorHeap* GetDescriptorHeap() { return descHeap; }
	private:
		// create
		void create(ID3D12Device* d3d12Device);
		// reset transient
		virtual void resetTransient();
	private:
		// instances to build
		Vector<D3D12_RAYTRACING_INSTANCE_DESC> instanceDesc;
		// bottom level as to build
		Vector<RaytracingGeomtry*> bottomLevelGeometries;
		// toplevel as
		ID3D12Resource* topLevelAs = nullptr;
		// toplevel scratch
		ID3D12Resource* topLevelScratch = nullptr;
		// instance buffer
		ID3D12Resource* instanceBuffer = nullptr;
		// sizes
		UINT64 topLevelSize = 0;
		UINT64 scratchSize = 0;
		UINT64 instanceSize = 0;
		// instance buffer pointer
		void* instancePtr = nullptr;
		// descripter heap
		D3D12DescriptorHeap* descHeap = nullptr;
		// shader binding table
		ShaderBindingTable sbt = {};
		// rtpso
		RaytracingStateObject stateObject = {};
		// device
		ID3D12Device* d3d12Device = nullptr;
		// rtxDevice
		ID3D12Device5* rtxDevice = nullptr;
	};

	/*
		ring constant buffer
	*/
	class RingConstantBuffer
	{
	public:
		// Alloc transient constant buffer
		void* AllocTransientConstantBuffer(unsigned int size, D3D12_GPU_VIRTUAL_ADDRESS* gpuAddress);
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
	class D3D12InputLayout : public ResourcePool<D3D12InputLayout, 512>
	{
	public:
		D3D12_INPUT_LAYOUT_DESC Layout;
		D3D12_INPUT_ELEMENT_DESC Element[32];
		char Names[32][32];

	};

	/* 
		shaders 
	*/
	class D3D12Shader : public ResourcePool<D3D12Shader, 512>
	{
	public:
		D3D12_SHADER_BYTECODE ByteCode;
		void* RawCode;
	};

	/*
		raytracing shader and collection
	*/
	class RaytracingShader : public ResourcePool<RaytracingShader, 512>
	{
	public:
		//
	public:
		// collection
		ID3D12StateObject* collection;
		// shader indentifier 
		ShaderIdetifier raygen;
		ShaderIdetifier hitGroup;
		ShaderIdetifier miss;
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
	class D3D12RenderState : public ResourcePool<D3D12RenderState, 512>
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

