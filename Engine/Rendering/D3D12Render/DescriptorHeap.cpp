#include "DescriptorHeap.h"
#include "D3D12Render.h"
#include <assert.h>


using namespace D3D12API;


List<DescriptorHeap> DescriptorHeap::CpuRetired[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
List<DescriptorHeap> DescriptorHeap::GpuRetired[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];


DescriptorHeap::DescriptorHeap(ID3D12Device * Device_, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flag) {
	Device = Device_;
	Type = type;
	Flag = flag;
	Size = MAX_DESCRIPTOR_SIZE;
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Flags = Flag;
	desc.Type = Type;
	desc.NodeMask = 0;
	desc.NumDescriptors = MAX_DESCRIPTOR_SIZE;
	HRESULT result = Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&Heap));
	Current = 0;
	Increment = Device->GetDescriptorHandleIncrementSize(type);
	CpuStart = Heap->GetCPUDescriptorHandleForHeapStart();
	GpuStart = Heap->GetGPUDescriptorHandleForHeapStart();
}

DescriptorHeap::~DescriptorHeap() {
}

DescriptorHeap * DescriptorHeap::Alloc(ID3D12Device * Device, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flag) {
	D3D12Render * Render = D3D12Render::GetRender();
	CommandQueue * Queue = Render->GetQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	List<DescriptorHeap> * retired = &CpuRetired[type];
	if (flag == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE) {
		retired = &GpuRetired[type];
	}
	DescriptorHeap * heap = NULL;
	List<DescriptorHeap>::Iterator Iter;
	for (Iter = retired->Begin(); Iter != retired->End(); Iter++) {
		heap = *Iter;
		if (Queue->FenceComplete(heap->FenceValue)) {
			retired->Remove(Iter);
			return heap;
		}
	}
	// create a new heap
	heap = new DescriptorHeap(Device, type, flag);
	heap->FenceValue = 0;
	return heap;
}

void DescriptorHeap::Retire(UINT64 FenceValue_) {
	FenceValue = FenceValue_;
	List<DescriptorHeap> & retired = CpuRetired[Type];
	if (Flag == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE) {
		retired = GpuRetired[Type];
	}
	retired.Insert(this);
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCpuHandle(int slot) {
	CD3DX12_CPU_DESCRIPTOR_HANDLE Handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(CpuStart, slot, Increment);
	return Handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGpuHandle(int slot) {
	CD3DX12_GPU_DESCRIPTOR_HANDLE Handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(GpuStart, slot, Increment);
	return Handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::StageDescriptors(D3D12_CPU_DESCRIPTOR_HANDLE * Handles, int Num) {
	// must be a shader visible heap
	assert(Flag == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

	int End = Current + Num;
	if (End > Size) {
		// not enough space
		return CD3DX12_GPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT());
	}
	// there is enough space
	CD3DX12_GPU_DESCRIPTOR_HANDLE GpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(GpuStart, Current, Increment);
	CD3DX12_CPU_DESCRIPTOR_HANDLE CpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(CpuStart, Current, Increment);
	Current += Num;
	// copy descriptors
	UINT DestRangeSize = 1;
	for (int i = 0; i < Num; i++) {
		SrcRangeSize[i] = 1;
		SrcStarts[i] = Handles[i];
	}
	Device->CopyDescriptors(1, &CpuHandle, &DestRangeSize, Num, SrcStarts, SrcRangeSize, Type);
	return GpuHandle;
}