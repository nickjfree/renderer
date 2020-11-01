#include "RenderQueue.h"
#include "Opcode.h"
#include <memory>

RenderQueue::RenderQueue()
{
	memset(CommandBuffers, 0, sizeof(void*) * R_STAGE_MAX);

	InitStageMessages();
}


RenderQueue::~RenderQueue()
{
}


void RenderQueue::InitStageMessages()
{
	// set all to zero
	memset(StageMessages, 0, R_STAGE_MAX * sizeof(char*));
	// shadow maps
	StageMessages[R_STAGE_SHADOW + 0] = "shadowmap0";
	StageMessages[R_STAGE_SHADOW + 1] = "shadowmap1";
	StageMessages[R_STAGE_SHADOW + 2] = "shadowmap2";
	StageMessages[R_STAGE_SHADOW + 3] = "shadowmap3";
	StageMessages[R_STAGE_SHADOW + 4] = "shadowmap4";
	StageMessages[R_STAGE_SHADOW + 5] = "shadowmap5";
	StageMessages[R_STAGE_SHADOW + 6] = "shadowmap6";
	StageMessages[R_STAGE_SHADOW + 7] = "shadowmap7";
	StageMessages[R_STAGE_SHADOW + 8] = "shadowmap8";
	// render to textures
	StageMessages[R_STAGE_RTT] = "render_to_texture";
	// prepass
	StageMessages[R_STAGE_PREPASSS] = "prepass";
	// oit
	StageMessages[R_STAGE_OIT] = "oit";
	// rt build
	// lighting
	StageMessages[R_STAGE_LIGHT] = "lighting";
	// rt dispatch
	StageMessages[R_STAGE_RT_DISPATCH2] = "raytracing";
	// post processing
	// StageMessages[R_STAGE_POST] = "post_processing";
}


int RenderQueue::PushCommand(int Stage, void* Commands) {
	if (!Commands || Stage >= R_STAGE_MAX || Stage < 0) {
		return -1;
	}
	CommandBuffers[Stage] = Commands;
	return Stage;
}

void* RenderQueue::PopCommand(int Stage) {
	if (Stage < 0 || Stage >= R_STAGE_MAX) {
		return 0;
	}
	void* ret = CommandBuffers[Stage];
	CommandBuffers[Stage] = 0;
	return ret;
}

int RenderQueue::Execute(RenderProcesser* Processer) {
	// set a present cmd
	//PushCommand(R_STAGE_PRESENT, Presnet);
	// execte all command buffers
	for (int Stage = 0; Stage < R_STAGE_MAX; Stage++) {
		void* CommandBuffer = CommandBuffers[Stage];
		if (CommandBuffer) {
			const char* message = StageMessages[Stage];
			Processer->Execute(CommandBuffer, 0xFF00FF00, message);
			// clear command buffer pointer. so old commands won't run again in the next frame 
			CommandBuffers[Stage] = 0;
		}
	}
	return 0;
}
