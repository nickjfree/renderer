#ifndef __LIGHTPRE_PROCESSER__
#define __LIGHTPRE_PROCESSER__
#include "renderprocesser.h"

namespace Render {

class CLightPreProcesser : public CProcesser
{
public:
	CLightPreProcesser(IMRender * Render);
	~CLightPreProcesser(void);
protected:
	// set rendersate , constants,shaders,effect
	virtual int Prepare(RenderState * State,RenderCommand * Command);
	// process a renderable
	virtual int ProcessRenderable(Renderable * pRenderable,int * ContextKey);
};

}

#endif