#ifndef __SHADOW_NORMAL_PROCESSER__
#define __SHADOW_NORMAL_PROCESSER__


#include "normalprocesser.h"

namespace Render {

class CShadowNormalProcesser : public CNormalProcesser
{
public:
	CShadowNormalProcesser(IMRender * Render);
	virtual ~CShadowNormalProcesser(void);

protected:
	// set rendersate , constants,shaders,effect
	virtual int Prepare(RenderState * State,RenderCommand * Command);
	// process a renderable
	virtual int ProcessRenderable(Renderable * pRenderable,int * ContextKey);

};

}

#endif