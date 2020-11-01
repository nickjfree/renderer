#ifndef __RENDER_QUEUE__
#define __RENDER_QUEUE__

#include "RenderProcesser.h"
/*
	render stage defines
*/
#define R_STAGE_MAX          0xFF
#define R_STAGE_SHADOW       0x00
#define R_STAGE_RTT          0x0F
#define R_STAGE_PREPASSS     0x10

#define R_STAGE_OIT          0x12

#define R_STAGE_SHADING      0x13
#define R_STAGE_HOC          0x14

#define R_STAGE_RT_BUILD     0xA0

#define R_STAGE_RT_DISPATCH1 0xA1

#define R_STAGE_LIGHT        0xA2

#define R_STAGE_RT_DISPATCH2 0xA3

#define R_STAGE_POST         0xA4

#define R_STAGE_DEBUG        0xFD


#define R_STAGE_PRESENT      0xFE



/*
	renderqueue, a collection of command buffers
*/

class RenderQueue
{
private:
	// commands
	void* CommandBuffers[R_STAGE_MAX];
	// stage messages
	char* StageMessages[R_STAGE_MAX];
private:
	// init stage messages
	void InitStageMessages();
public:
	RenderQueue();
	virtual ~RenderQueue();
	// add render commands to certain bucket
	int PushCommand(int Stage, void* Commands);
	// out queue
	void* PopCommand(int Stage);
	// execute
	virtual int Execute(RenderProcesser* Processer);
};


#endif