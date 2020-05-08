#ifndef __REUSE_HEAP__
#define __REUSE_HEAP__


#include "Heap.h"


namespace D3D12API {

	class ReuseHeap
	{
	private:
		// heap 
		Heap * heap_;
		// type
		int Type_;
		// MaxSize
		UINT64 MaxSize_;
		// resource state
		D3D12_RESOURCE_STATES State_;
		// resource flag
		D3D12_RESOURCE_FLAGS Flag_;
		// d3d12 Device
		ID3D12Device* Device_;
	private:
		void AllocHeap(UINT64 MaxSize, int Type);
	public:
		ReuseHeap(ID3D12Device* Device, int MaxSize, int Type, D3D12_RESOURCE_STATES State = D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_FLAGS Flag = D3D12_RESOURCE_FLAG_NONE);
		~ReuseHeap();
		// Alloc 
		void* EnsureSize(UINT64 Size);
		// get resource
		ID3D12Resource* GetResource();

	};

}

#endif