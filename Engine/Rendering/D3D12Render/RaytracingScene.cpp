
#include "RaytracingScene.h"

using namespace::D3D12API;


RaytracingScene::RaytracingScene(): SceneFenceValue_(-1) {

}
RaytracingScene::~RaytracingScene() {

}

/*
	build top level as in cmdContext. with an async compute queue
*/
UINT64 RaytracingScene::BuildTopLevelAccelerationStructure(CommandContext* cmdContext) {
	
	// build top level as
	
	// do nothing for now
	SceneFenceValue_ = cmdContext->Flush(0);
	return SceneFenceValue_;
}

UINT64 RaytracingScene::BuildBottomLevelAccelerationStructure(CommandContext* cmdContext) {
	// do nothing for now
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