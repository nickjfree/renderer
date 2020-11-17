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

	/*
		command context
	*/
	class D3D12CommandContext : public Transient<D3D12CommandContext>
	{
		friend Transient<D3D12CommandContext>;
	public:

	private:
		// reset transient resource status
		void resetTransient() {};
	private:
		// context type
		enum class COMMAND_CONTEXT_TYPE {
			GRAPHIC,
			COMPUTE,
			COPY,
			COUNT,
		};
		// command list
		ID3D12CommandList* cmdList;
		// command allocator
		ID3D12CommandAllocator* CommandAllocator;
		// rt commandlist
		ID3D12GraphicsCommandList5* rtCommandList;
	};


	/*
		Descriptor Heap
	*/
	class D3D12DescriptorHeap : public Transient<D3D12DescriptorHeap>
	{
	private:
		// reset
		void resetTransient() {};
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
	private:
		// device
		ID3D12Device * d3d12Device;
	};


}

#endif