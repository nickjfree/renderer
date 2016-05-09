#ifndef __RENDER_QUEUE__
#define __RENDER_QUEUE__

#include "RenderProcesser.h"
/*
	render stage defines
*/
#define R_STAGE_MAX      0xFF
#define R_STAGE_SHADOW   0x00
#define R_STAGE_PREPASSS 0x10
#define R_STAGE_LIGHT    0x11
#define R_STAGE_SHADING  0x12
#define R_STAGE_HOC      0x13
#define R_STAGE_POST     0xA0


/*
	renderqueue, a collection of command buffers
*/

class RenderQueue
{
private:
	void * CommandBuffers[R_STAGE_MAX];
public:
	RenderQueue();
	virtual ~RenderQueue();
	// add render commands to certain bucket
	int PushCommand(int Stage, void * Commands);
	// out queue
	void * PopCommand(int Stage);
	// execute
	virtual int Execute(RenderProcesser * Processer);
};


#endif