#ifndef __D3D12_RESOURCE__
#define __D3D12_RESOURCE__

#include "D3D12Renderer.h"

namespace D3D12Renderer {


	/*
		resource types
	*/
	enum class RESOURCE_TYPES {
		BUFFER,
		TEXTURE,
		GEOMETRY,
		BLAS,
		TLAS,
	};

	/*
		descripter type
	*/
	enum class DESCRIPTOR_HANDLE_TYPES {
		SRV,
		UAV,
		RTV,
		DSV,
		COUNT,
	};

	/*
		directx resource base class
	*/
	class D3D12Resource
	{
	public:		
		// set state (issue a transfer barrier)
		void SetResourceState(ID3D12CommandList* cmdList, D3D12_RESOURCE_STATES targetState);
		// srv
		D3D12_GPU_DESCRIPTOR_HANDLE  GetSrv();
		// uav
		D3D12_GPU_DESCRIPTOR_HANDLE  GetUav();
		// rtv
		D3D12_CPU_DESCRIPTOR_HANDLE  GetRtv();
		// dsv
		D3D12_CPU_DESCRIPTOR_HANDLE  GetDsv();
	public:
		// resource type
		RESOURCE_TYPES resourceType;
		// resource state
		D3D12_RESOURCE_STATES state;
		// srv
		D3D12_CPU_DESCRIPTOR_HANDLE  views[(int)DESCRIPTOR_HANDLE_TYPES::COUNT];
	private: 
		// resource index
		unsigned int index;

	};

	/*
		buffer (shader resource or uav)
	*/
	class BufferResource : public D3D12Resource
	{
	};

	/*
		texture ( render targets, shader resource or uav)
	*/
	class TextureResource : public D3D12Resource
	{
	};

	/*
		vertexbuffer + indexbuffer
	*/
	class Geometry
	{
	public:

	private:
		BufferResource* vertextBuffer;
		BufferResource* indexBuffer;
	};

}


#endif

