#include "Heap.h"
#include "D3D12Render.h"
#include <assert.h>

using namespace D3D12API;


Vector<Heap*> Heap::Free;
List<Heap> Heap::Retired;

Heap::Heap() :CpuData(0), CurrentOffset(0), Buffer(0) {
}

Heap::~Heap() {
	if (CpuData) {
		Buffer->Unmap(0, NULL);
	}
	if (Buffer) {
		Buffer->Release();
	}
}

void Heap::Init(ID3D12Device* Device_, int MaxSize_, int Type_) {
	assert(Type_ == HeapType::CPU);
	D3D12_HEAP_TYPE HeapType;
	Device = Device_;
	Type = Type_;
	MaxSize = MaxSize_;
	if (Type == HeapType::CPU) {
		HeapType = D3D12_HEAP_TYPE_UPLOAD;
	}
	else {
		HeapType = D3D12_HEAP_TYPE_DEFAULT;
	}
	// create the heap
	HRESULT result = Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(HeapType),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(MaxSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&Buffer));
	if (Type == HeapType::CPU) {
		CD3DX12_RANGE readRange(0, 0);
		Buffer->Map(0, &readRange, &CpuData);
		GpuData = Buffer->GetGPUVirtualAddress();
	}
}

Heap* Heap::Alloc(ID3D12Device* Device, int Type) {
	Heap* heap = 0;
	D3D12Render* render = D3D12Render::GetRender();
	CommandQueue* Queue = render->GetQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	if (Free.Size()) {
		heap = Free.PopBack();
	}
	else {
		bool Found = 0;
		for (auto Iter = Retired.Begin(); Iter != Retired.End(); Iter++) {
			heap = *Iter;
			UINT64 FenceValue = heap->FenceValue;
			if (Queue->FenceComplete(FenceValue)) {
				Retired.Remove(Iter);
				Found = 1;
				break;
			}
		}
		if (!Found) {
			heap = 0;
		}
	}
	if (heap) {
		return heap;
	}
	else {
		// new context
		heap = new Heap();
		heap->Init(Device, MAX_CONSTANT_BUFFER_HEAP, Type);
	}
	return heap;
}

void* Heap::SubAlloc(int Size) {
	int offset = 0;
	int AlignedSize = (Size + CONSTANT_ALIGN - 1) & ~(CONSTANT_ALIGN - 1);
	if (CurrentOffset + Size <= MaxSize) {
		UINT64 pCurrent = (UINT64)CpuData + CurrentOffset;
		CurrentOffset += AlignedSize;
		return (void*)pCurrent;
	}
	return NULL;
}

D3D12_GPU_VIRTUAL_ADDRESS Heap::GetGpuAddress(void* CpuPointer) {
	UINT64 offset = (UINT64)CpuPointer - (UINT64)CpuData;
	return GpuData + offset;
}

void Heap::Retire(UINT64 FenceValue_) {
	FenceValue = FenceValue_;
	CurrentOffset = 0;
	Retired.Insert(this);
}

void Heap::Release() {
	Heap* heap = this;
	Free.PushBack(heap);
}
