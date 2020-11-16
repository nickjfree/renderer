#ifndef __D3D12_RENDERER__
#define __D3D12_RENDERER__

#include "windows.h"
#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_4.h>
#include "pix3.h"
#include "Rendering/RenderInterface.h"
#include "Rendering/RenderDesc.h"
#include "Tasks/Mutex.h"
#include "Container/List.h"
#include "Container/Vector.h"

namespace D3D12Renderer {


	/*
		transient item pool
	*/
	template <class ItemType>
	class Transient
	{
	public:
		// alloc item 
		static ItemType* Alloc();
		// retire all
		static unsigned int RetireAll(unsigned int fence);
	private:
		// retire
		void retire(unsigned int fence);
	private:
		// lock
		static Mutex  lock;
		// retired pool
		static List<ItemType> retired;
		// vector pending item to retire
		static Vector<ItemType*> retiring;
		// fence
		unsigned int fenceValue = 0;
	};

	// initalization of the retired pool
	template <class ItemType> List<ItemType> Transient<ItemType>::retired;
	// initalization of the retiring pool
	template <class ItemType> Vector<ItemType*> Transient<ItemType>::retiring;
	//  initalization of the lock
	template <class ItemType> Mutex Transient<ItemType>::lock;


	// alloc an item
	template <class ItemType> ItemType * Transient<ItemType>::Alloc()
	{
		// TODO: 
		// 1. get graphic queue fenceComplete value
		// 2. find item from retired pool

		// 3. can not found a retired item from pool
		// call init
		auto item = new ItemType();
		// 4. add item to pending pool
		lock.Acquire();
		retiring.PushBack(item);
		lock.Release();
		return item;
	}

	// retire all items
	template <class ItemType> unsigned int Transient<ItemType>::RetireAll(unsigned int fence)
	{
		lock.Acquire();
		for(auto iter = retiring.Begin(); iter != retiring.End(); iter++) {
			(*iter).retire(fence);
		}
		// clear all retiring items
		retiring.Reset();
		lock.Release();
		return item;
	}

	// retire item
	template <class ItemType> void Transient<ItemType>::retire(unsigned int fence)
	{
		fenceValue = fence;
		retired.Insert(this);
	}

	/*
		command context
	*/
	class D3D12CommandContext : public Transient<D3D12CommandContext>
	{
	public:

	private:
		// create new command context
		void init();
	private:
		// context type
		enum class COMMAND_CONTEXT_TYPE {
			GRAPHIC,
			COMPUTE,
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
		the render interface
	*/
	class D3D12RenderInterface : public RenderInterface
	{

	};


}

#endif