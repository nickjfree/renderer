#ifndef __D3D12_HELPER__
#define __D3D12_HELPER__

#include <functional>
#include "Container/List.h"
#include "Container/Vector.h"
#include "Tasks/Mutex.h"




namespace D3D12Renderer {

	class D3D12CommandQueue;

	/*
		transient item pool
	*/
	template <class ItemType>
	class Transient
	{
	public:
		// retire all
		static unsigned int RetireAll(UINT64 fence);
	protected:
		// alloc item 
		static ItemType* allocTransient(std::function<void(ItemType*)> createCallback, std::function<bool(ItemType*)> matchCallback);
	private:
		// retire
		void retire(UINT64 fence);
		// reset transient resource status
		virtual void resetTransient() = 0;
	private:
		// lock
		static Mutex  lock;
		// retired pool
		static List<ItemType> retired;
		// vector pending item to retire
		static Vector<ItemType*> inflight;
		// fence
		UINT64 fenceValue = 0;
	};

	// initalization of the retired pool
	template <class ItemType> List<ItemType> Transient<ItemType>::retired;
	// initalization of the inflight pool
	template <class ItemType> Vector<ItemType*> Transient<ItemType>::inflight;
	//  initalization of the lock
	template <class ItemType> Mutex Transient<ItemType>::lock;

	/*
		alloc an transient item
	*	createCallback: create resource when not found	
	*   matchCallback:  match an item in the retired pool
	*/
	template <class ItemType> ItemType* Transient<ItemType>::allocTransient(
		std::function<void(ItemType*)> createCallback, 
		std::function<bool(ItemType*)> matchCallback)
	{	
		// 1. get graphic queue fenceComplete value
		// UINT64 currentFence = 100;	
		// 2. find item from retired pool
		ItemType* item = nullptr;
		bool found = false;
		lock.Acquire();
		for (auto iter = retired.Begin(); iter != retired.End(); iter++) {
			item = *iter;
			auto queue = D3D12CommandQueue::GetQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
			if (queue->IsFenceComplete(item->fenceValue) && matchCallback(item)) {
				retired.Remove(iter);
				item->resetTransient();
				found = true;
				break;
			}
		}
		lock.Release();
		// 3. can not found a retired item from pool
		if (!found) {
			item = new ItemType();
			// call create callback
			createCallback(item);
		}
		// 4. add item to inflight pool
		lock.Acquire();
		inflight.PushBack(item);
		lock.Release();
		return item;
	}

	// retire all items
	template <class ItemType> unsigned int Transient<ItemType>::RetireAll(UINT64 fence)
	{
		lock.Acquire();
		for (auto iter = inflight.Begin(); iter != inflight.End(); iter++) {
			(*iter)->retire(fence);
		}
		auto numRetired = inflight.Size();
		// clear all retiring items
		inflight.Reset();
		lock.Release();
		return numRetired;
	}

	// retire item
	template <class ItemType> void Transient<ItemType>::retire(UINT64 fence)
	{
		fenceValue = fence;
		retired.Insert(static_cast<ItemType*>(this));
	}


	/*
		resource pool
	*/
	template <class ResType, int size>
	class ResourcePool
	{
	public:
		// alloc resource
		static ResType* Alloc();
		// get
		static ResType* Get(int id);
		// free resource
		void Free();
	public:
		// resource id
		int resourceId;
	private:
		// lock
		static Mutex lock;
		// resource pool
		static ResType resources[size];
		// free id list
		static Vector<int> freeList;
		// current index
		static int currentIndex;
	};

	// initialize members of the resource pool
	template <class ResType, int size> Mutex ResourcePool<ResType, size>::lock;
	template <class ResType, int size> ResType ResourcePool<ResType, size>::resources[size];
	template <class ResType, int size> Vector<int> ResourcePool<ResType, size>::freeList;
	template <class ResType, int size> int ResourcePool<ResType, size>::currentIndex;


	// alloc resource
	template <class ResType, int size> ResType* ResourcePool<ResType, size>::Alloc()
	{
		ResType* ret;
		if (freeList.Size()) {
			// get from  free list
			auto id = freeList.PopBack();
			ret = &resources[id];
			ret->resourceId = id;
		}
		else {
			// alloc a new one
			ret = &resources[currentIndex];
			ret->resourceId = currentIndex++;
		}
		return ret;
	}

	// free resource
	template <class ResType, int size> void ResourcePool<ResType, size>::Free()
	{
		auto index = 0x00ffffff & resourceId;
		freeList.PushBack(index);
	}

	// get resource
	template <class ResType, int size> ResType* ResourcePool<ResType, size>::Get(int id)
	{
		// get resource index
		auto index = 0x00ffffff & id;
		return &resources[index];
	}


} // end namespace


#endif