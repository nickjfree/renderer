#include "RenderQueue.h"
#include <memory>

RenderQueue::RenderQueue()
{
	memset(CommandBuffers, 0, sizeof(void*)*R_STAGE_MAX);
}


RenderQueue::~RenderQueue()
{
}


int RenderQueue::PushCommand(int Stage, void * Commands) {
	if (!Commands || Stage >= R_STAGE_MAX || Stage < 0) {
		return -1;
	}
	CommandBuffers[Stage] = Commands;
	return Stage;
}

void * RenderQueue::PopCommand(int Stage) {
	if (Stage < 0 || Stage >= R_STAGE_MAX) {
		return 0;
	}
	void * ret = CommandBuffers[Stage];
	CommandBuffers[Stage] = 0;
	return ret;
}

int RenderQueue::Execute(RenderProcesser * Processer) {
	// execte all command buffers
	for (int Stage = 0; Stage < R_STAGE_MAX; Stage++) {
		void * CommandBuffer = CommandBuffers[Stage];
		if (CommandBuffer) {
			Processer->Execute(CommandBuffer);
			// clear command buffer pointer. so old commands won't run again in the next frame 
			CommandBuffers[Stage] = 0;
		}
	}
	return 0;
}
