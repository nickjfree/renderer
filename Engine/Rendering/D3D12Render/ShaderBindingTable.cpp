
#include "ShaderBindingTable.h"


using namespace::D3D12API;

ShaderBindingTable::ShaderBindingTable(ID3D12Device* Device):Size_(8192)
{
	UploadBuffer = new ReuseHeap(Device, Size_, Heap::HeapType::CPU, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_FLAG_NONE);
	GPUBuffer = new ReuseHeap(Device, Size_, Heap::HeapType::GPU, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_FLAG_NONE);
}

ShaderBindingTable::~ShaderBindingTable()
{
	if (UploadBuffer) {
		delete UploadBuffer;
	}
	if (GPUBuffer) {
		delete GPUBuffer;
	}
}

void ShaderBindingTable::EnsureSize(UINT Size)
{
	if (Size > Size_) {
		// alloc more resource
		UploadBuffer->EnsureSize(Size);
		GPUBuffer->EnsureSize(Size);
		Size_ = Size;
	}
}