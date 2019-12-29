
#include "RaytracingScene.h"

using namespace::D3D12API;


RaytracingScene::RaytracingScene(ID3D12Device* Device): SceneFenceValue_(-1), Device_(Device) {
	Device_->QueryInterface(IID_PPV_ARGS(&rtDevice_));
	// create toplevle resource
	TopLevelAS = new ReuseHeap(Device_, 1024, Heap::HeapType::GPU, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	// scratch resource
	TopLevelScratch = new ReuseHeap(Device_, 1024, Heap::HeapType::GPU, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	// instance resource
	InstancesBuffer = new ReuseHeap(Device_, 1024, Heap::HeapType::CPU, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_FLAG_NONE);
}

RaytracingScene::~RaytracingScene() {

}

/*
	build top level as in cmdContext. with an async compute queue
*/
UINT64 RaytracingScene::BuildTopLevelAccelerationStructure(CommandContext* cmdContext) {
	
	// build top level as

	// 1. copy instance desc to upload buffer (InstancesBuffer)
	if (InstanceDesc.Size()) {

		auto instanceSize = InstanceDesc.Size() * sizeof(D3D12_RAYTRACING_INSTANCE_DESC);
		auto CPUData = InstancesBuffer->Alloc(instanceSize);
		memcpy(CPUData, InstanceDesc.GetData(), instanceSize);
	}
	// 2. get toplevel size
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {};
	topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	topLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	topLevelInputs.NumDescs = InstanceDesc.Size();
	topLevelInputs.pGeometryDescs = nullptr;
	topLevelInputs.ppGeometryDescs = nullptr;
	topLevelInputs.InstanceDescs = InstancesBuffer->GetResource()->GetGPUVirtualAddress();
	topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
	
	rtDevice_->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);

	// 3. reallocate top level data size
	TopLevelScratch->Alloc(topLevelPrebuildInfo.ScratchDataSizeInBytes);
	TopLevelAS->Alloc(topLevelPrebuildInfo.ResultDataMaxSizeInBytes);

	// 4. build top level as
	auto cmdList = cmdContext->GetRaytracingCommandList();

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
	buildDesc.DestAccelerationStructureData = TopLevelAS->GetResource()->GetGPUVirtualAddress();
	buildDesc.ScratchAccelerationStructureData = TopLevelScratch->GetResource()->GetGPUVirtualAddress();
	buildDesc.SourceAccelerationStructureData = 0;
	buildDesc.Inputs = topLevelInputs;
	cmdList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

	// 5. uav barriar
	CD3DX12_RESOURCE_BARRIER UAVBarriers[2] = { CD3DX12_RESOURCE_BARRIER::UAV(TopLevelAS->GetResource()),  CD3DX12_RESOURCE_BARRIER::UAV(TopLevelScratch->GetResource())};
	cmdList->ResourceBarrier(2, UAVBarriers);

	// flush context
	SceneFenceValue_ = cmdContext->Flush(0);

	// clear instances
	InstanceDesc.Empty();
	return SceneFenceValue_;
}

UINT64 RaytracingScene::BuildBottomLevelAccelerationStructure(CommandContext* cmdContext) {
	// do nothing for now
	// TODO: rebuild all bottom level as for deformable geometries
	return cmdContext->Finish(0);
}

UINT64 RaytracingScene::WaitScene(CommandContext* GraphicContext) {
	// wait for compute queue to reach FenceValue.
	// so the scene build is finished
	if (SceneFenceValue_ == -1) {
		return -1;
	}
	// wait for a compute fencevalue in graphic queue
	auto GraphicFence = GraphicContext->WaitQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE, SceneFenceValue_);
	SceneFenceValue_ = -1;
	return GraphicFence;
}


void RaytracingScene::AddInstance(ID3D12Resource* BottomLevelAs, UINT InstanceID, UINT Flags, Matrix4x4& Tansform) {
	D3D12_RAYTRACING_INSTANCE_DESC instance = {};
	instance.AccelerationStructure = BottomLevelAs->GetGPUVirtualAddress();
	instance.InstanceID = InstanceID;
	instance.Flags = Flags;
	// set transform to indetity
	instance.Transform[0][0] = instance.Transform[1][1] = instance.Transform[2][2] = 1;
	InstanceDesc.PushBack(instance);
}
