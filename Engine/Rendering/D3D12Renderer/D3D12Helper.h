#ifndef __D3D12_HELPER__
#define __D3D12_HELPER__

#include "Container/List.h"
#include "Container/Vector.h"
#include "Tasks/Mutex.h"




namespace D3D12Renderer {

	/*
		transient item pool
	*/
	template <class ItemType>
	class Transient
	{
	public:
		// alloc item 
		static ItemType* AllocTransient();
		// retire all
		static unsigned int RetireAll(unsigned int fence);
	
	private:
		// retire
		void retire(unsigned int fence);
		// reset transient resource status
		virtual void resetTransient() = 0;
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


	// alloc an transient item
	template <class ItemType> ItemType* Transient<ItemType>::AllocTransient()
	{
		// TODO: 
		// 1. get graphic queue fenceComplete value
		// 2. find item from retired pool

		// 3. can not found a retired item from pool
		// call init
		auto item = new ItemType();
		if (true) { // if found
			item->resetTransient();
		}
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
		for (auto iter = retiring.Begin(); iter != retiring.End(); iter++) {
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