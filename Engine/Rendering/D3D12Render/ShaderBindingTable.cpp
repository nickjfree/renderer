
#include "ShaderBindingTable.h"


using namespace::D3D12API;

ShaderBindingTable::ShaderBindingTable(ID3D12Device* Device):Size_(8192)
{
	UploadBuffer = new ReuseHeap(Device, Size_, Heap::HeapType::CPU, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_FLAG_NONE);
	GPUBuffer = new ReuseHeap(Device, Size_, Heap::HeapType::GPU, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_FLAG_NONE);
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
	int Index = HitGroup.PushBack(Record);
	return &HitGroup[Index];
}

void ShaderBindingTable::UpdateRay(int rayIndex, D3DShaderIdetifier& raygenIdentifier, D3DShaderIdetifier& missIdentifier) {
	RayGen[rayIndex].ShaderIdentifier = raygenIdentifier;
	Miss[rayIndex].ShaderIdentifier = missIdentifier;
}

void ShaderBindingTable::Reset() {
	HitGroup.Empty();
	Current = 0;
}

void ShaderBindingTable::Stage(CommandContext* cmdContext) {
	// ensure total resource size
	auto hitGroupSize = HitGroup.Size() * sizeof(ShaderRecord);
	EnsureSize(hitGroupSize + HITGROUP_TABLE_OFFSET);
	auto cmdList = cmdContext->GetGraphicsCommandList();
	// copy resource to upload heap
	void* cpuData;
	auto uploadHeap = UploadBuffer->GetResource();
	uploadHeap->Map(0, nullptr, &cpuData);
	// copy raygen
	memcpy((char*)cpuData + RAYGEN_TABLE_OFFSET, RayGen, RAYGEN_TABLE_SIZE);
	// copy miss
	memcpy((char*)cpuData + MISS_TABLE_OFFSET, Miss, MISS_TABLE_SIZE);
	// copy hitgroups
	memcpy((char*)cpuData + HITGROUP_TABLE_OFFSET, HitGroup.GetData(), hitGroupSize);
	uploadHeap->Unmap(0, nullptr);
	// copy to sbt shader visible buffer
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GPUBuffer->GetResource(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
	cmdList->CopyResource(GPUBuffer->GetResource(), uploadHeap);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(GPUBuffer->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
	// update table pointers
	auto tableAddress = GPUBuffer->GetResource()->GetGPUVirtualAddress();
	
	memset(&RayDesc, 0, sizeof(RayDesc));

	RayDesc.RayGenerationShaderRecord.StartAddress = tableAddress + RAYGEN_TABLE_OFFSET;
	RayDesc.RayGenerationShaderRecord.SizeInBytes = 64;

	RayDesc.MissShaderTable.StartAddress = tableAddress + MISS_TABLE_OFFSET;
	RayDesc.MissShaderTable.SizeInBytes = MISS_TABLE_SIZE;
	RayDesc.MissShaderTable.StrideInBytes = sizeof(ShaderRecord);

	RayDesc.HitGroupTable.StartAddress = tableAddress + HITGROUP_TABLE_OFFSET;
	RayDesc.HitGroupTable.SizeInBytes = hitGroupSize;
	RayDesc.HitGroupTable.StrideInBytes = sizeof(ShaderRecord);

	RayDesc.Depth = 1;
}