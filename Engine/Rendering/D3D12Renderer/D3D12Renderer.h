#ifndef __D3D12_RENDERER__
#define __D3D12_RENDERER__

#include "D3D12Common.h"
#include "D3D12Resource.h"
#include "Tasks/Mutex.h"
#include "Container/List.h"
#include "Container/Vector.h"


namespace D3D12Renderer {

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
		// create geometry
		int CreateGeometry(R_GEOMETRY_DESC* desc);
		// destory geometry
		int DestoryGeometry(int id);
	private:
		// get resource
		D3D12Resource* GetResource(int id);
		// destory resource
		void DestoryResource(int id);
		// create window
		HWND CreateRenderWindow(int width, int height);
		// init descriptor heaps
		void InitDescriptorHeaps();
		// init cmdQueue
		void InitCommandQueues();
		// init d3d12Device
		void InitD3D12Device();
		// init backbuffer
		void InitBackBuffer(int width, int height);
	private:
		// DXGIFactory
		IDXGIFactory4* dxgiFactory = nullptr;
		// device
		ID3D12Device* d3d12Device = nullptr;
		// rtxDevice
		ID3D12Device5* rtxDevice = nullptr;
		// descriptor heaps
		D3D12DescriptorHeap* descHeaps[(unsigned int)D3D12DescriptorHeap::DESCRIPTOR_HANDLE_TYPES::COUNT];
		// backbuffer
		D3D12BackBuffer backBuffer;
	};
}

#endif