
#include "ReuseHeap.h"


using namespace D3D12API;


ReuseHeap::ReuseHeap(ID3D12Device* Device, int MaxSize, int Type, D3D12_RESOURCE_STATES State, D3D12_RESOURCE_FLAGS Flag): State_(State), Flag_(Flag) {
	// device
	Device_ = Device;
	AllocHeap(MaxSize, Type);
}

ReuseHeap::~ReuseHeap() {
	delete heap_;
}

void ReuseHeap::AllocHeap(UINT64 MaxSize, int Type) {
	heap_ = new Heap();
	heap_->Init(Device_, MaxSize, Type, State_, Flag_);
	// size
	MaxSize_ = MaxSize;
	// type
	Type_ = Type;
}

void* ReuseHeap::Alloc(UINT64 Size) {
	if (Size <= MaxSize_) {
		return heap_->GetCPUAddress(0);
	} else {
		// need to alloce a bigger one
		delete heap_;
		AllocHeap(Size, Type_);
		return heap_->GetCPUAddress(0);
	}
}

ID3D12Resource* ReuseHeap::GetResource() {
	return heap_->GetResource();
}