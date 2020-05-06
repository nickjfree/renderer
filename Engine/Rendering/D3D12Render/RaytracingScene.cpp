
#include "RaytracingScene.h"
#include "D3D12Render.h"

using namespace::D3D12API;


// retired scene
List<RaytracingScene> RaytracingScene::RetiredScene;

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
	if (TopLevelAS) {
		delete TopLevelAS;
	}
	if (TopLevelScratch) {
		delete TopLevelScratch;
	}
	if (InstancesBuffer) {
		delete InstancesBuffer;
	}
}

// alloc new raytracing scene
RaytracingScene* RaytracingScene::Alloc(ID3D12Device* Device) {
	
	RaytracingScene* Result = nullptr;
	
	D3D12Render* Render = D3D12Render::GetRender();
	CommandQueue* Queue = Render->GetQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);

	for (auto iter = RetiredScene.Begin(); iter != RetiredScene.End(); iter++) {
		Result = *iter;
		if (Queue->FenceComplete(Result->SceneFenceValue_)) {
			RetiredScene.Remove(iter);
			Result->SceneFenceValue_ = -1;
			return Result;
		}
	}
	// no retired scenc founc 
	Result = new RaytracingScene(Device);
	return Result;
}
// retire raytracing scene
void RaytracingScene::Retire(UINT64 FenceValue) {
	SceneFenceValue_ = FenceValue;
	RetiredScene.Insert(this);
}

/*
	build top level as in cmdContext. with an async compute queue
*/
UINT64 RaytracingScene::BuildTopLevelAccelerationStructure(CommandContext* cmdContext) {
	
	// build top level as

	// 1. copy instance desc to upload buffer (InstancesBuffer)
	if (InstanceDesc.Size()) {

		auto instanceSize = InstanceDesc.Size() * sizeof(D3D12_RAYTRACING_INSTANCE_DESC);
		auto CPUData = InstancesBuffer->EnsureSize(instanceSize);
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
	TopLevelScratch->EnsureSize(topLevelPrebuildInfo.ScratchDataSizeInBytes);
	TopLevelAS->EnsureSize(topLevelPrebuildInfo.ResultDataMaxSizeInBytes);

	// 4. build top level as
	auto cmdList = cmdContext->GetRaytracingCommandList();

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
	buildDesc.DestAccelerationStructureData = TopLevelAS->GetResource()->GetGPUVirtualAddress();
	buildDesc.ScratchAccelerationStructureData = TopLevelScratch->GetResource()->GetGPUVirtualAddress();
	buildDesc.SourceAccelerationStructureData = 0;
	buildDesc.Inputs = topLevelInputs;
	auto i = 1;
	while (i--) {
		cmdList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

		// 5. uav barriar
		CD3DX12_RESOURCE_BARRIER UAVBarriers[2] = { CD3DX12_RESOURCE_BARRIER::UAV(TopLevelAS->GetResource()),  CD3DX12_RESOURCE_BARRIER::UAV(TopLevelScratch->GetResource()) };
		cmdList->ResourceBarrier(2, UAVBarriers);
	}

	// flush context
	SceneFenceValue_ = cmdContext->Finish(0);

	// clear instances
	InstanceDesc.Empty();
	return SceneFenceValue_;
}

UINT64 RaytracingScene::BuildBottomLevelAccelerationStructure(CommandContext* cmdContext, UINT64 GraphicsFenceValue) {
	// rebuild all bottom level as for deformable geometries
	auto cmdList = cmdContext->GetRaytracingCommandList();
	// wait for frev frame's  graphic work to complete
	if (GraphicsFenceValue == 0) {
		// we are the first frame
		BottomLevelDesc.Empty();
		return 0;
	}
	// wait for prev graphics work to finish
	cmdContext->WaitQueue(D3D12_COMMAND_LIST_TYPE_DIRECT, GraphicsFenceValue);
	for (auto Iter = BottomLevelDesc.Begin(); Iter != BottomLevelDesc.End(); Iter++) {
		auto& bottoemLevelDesc = *Iter;
		auto FrameIndex = bottoemLevelDesc.FrameIndex;
		D3D12_RESOURCE_STATES NewState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER|D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		//D3D12_RESOURCE_STATES NewState = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		D3D12_RESOURCE_STATES OldState = bottoemLevelDesc.Buffer->State[FrameIndex].CurrentState;
		if (OldState != NewState) {
			if (OldState == D3D12_RESOURCE_STATE_COPY_DEST) {
				// buffer was not deformed 
				BottomLevelDesc.Empty();
				return 0;
			}
			CD3DX12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(bottoemLevelDesc.Buffer->BufferResource[FrameIndex], OldState, NewState);
			ResourceBarriers.PushBack(Barrier);
			bottoemLevelDesc.Buffer->State[FrameIndex].CurrentState = NewState;
		}
	}
	// submit resource barriers for deformable buffer, uav --> vertex buffer
	if (ResourceBarriers.Size()) {
		cmdList->ResourceBarrier(ResourceBarriers.Size(), ResourceBarriers.GetData());
		ResourceBarriers.Empty();
	}
	// build them
	for (auto Iter = BottomLevelDesc.Begin(); Iter != BottomLevelDesc.End(); Iter++) {
		auto& bottoemLevelDesc = *Iter;
		auto FrameIndex = bottoemLevelDesc.FrameIndex;
		cmdList->BuildRaytracingAccelerationStructure(&bottoemLevelDesc.buildDesc, 0, nullptr);
		auto Barrier = CD3DX12_RESOURCE_BARRIER::UAV(bottoemLevelDesc.Blas->BLAS[FrameIndex]);
		ResourceBarriers.PushBack(Barrier);
		Barrier = CD3DX12_RESOURCE_BARRIER::UAV(bottoemLevelDesc.Blas->Scrach[FrameIndex]);
		ResourceBarriers.PushBack(Barrier);
		// clear dirty flag. so the next frame can use it to build the TLAS
		bottoemLevelDesc.Blas->Dirty[FrameIndex] = false;
	}
	// submit uav resource barriers for blas and scratch
	if (ResourceBarriers.Size()) {
		cmdList->ResourceBarrier(ResourceBarriers.Size(), ResourceBarriers.GetData());
		ResourceBarriers.Empty();
	}
	// clear deformable geometry
	BottomLevelDesc.Empty();
	return cmdContext->Flush(0);
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
	Matrix4x4 Trans;
	Tansform.Tranpose(Tansform, &Trans);
	memcpy(instance.Transform, &Trans, sizeof(float) * 12);
	//instance.Transform[0][0] = instance.Transform[1][1] = instance.Transform[2][2] = 1;
	InstanceDesc.PushBack(instance);
}


void RaytracingScene::RebuildBottomLevelAs(D3DBottomLevelAS* Blas, D3DBuffer* Buffer, D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC& buildDesc, int FrameIndex) {
	BottomLevelAsRebuildDesc desc {
		Blas,
		Buffer,
		buildDesc,
		FrameIndex
	};
	BottomLevelDesc.PushBack(desc);
	// mark blas as dirty
	Blas->Dirty[FrameIndex] = true;
}