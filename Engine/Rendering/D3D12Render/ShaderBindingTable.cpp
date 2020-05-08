
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

ShaderRecord* ShaderBindingTable::AllocRecord(int MaterialId) {

	// we ignore materialId for now
	ShaderRecord Record{};
	int Index = Data.PushBack(Record);
	return &Data[Index];
}


void ShaderBindingTable::Reset() {
	Data.Empty();
	Current = 0;
}

void ShaderBindingTable::Stage(CommandContext* cmdContext) {
	// ensure total resource size
	auto size = Data.Size() * sizeof(ShaderRecord);
	EnsureSize(size);
	auto cmdList = cmdContext->GetGraphicsCommandList();
	// copy resource to upload heap
	void* cpuData;
	auto uploadHeap = UploadBuffer->GetResource();
	uploadHeap->Map(0, nullptr, &cpuData);
	memcpy(cpuData, Data.GetData(), size);
	uploadHeap->Unmap(0, nullptr);
	// copy to sbt shader visible buffer
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GPUBuffer->GetResource(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST));
	cmdList->CopyResource(GPUBuffer->GetResource(), uploadHeap);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GPUBuffer->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
}