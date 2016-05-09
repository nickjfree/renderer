#ifndef __LIGHT_PROCESSER__
#define __LIGHT_PROCESSER__

#include "RenderProcesser.h"

namespace Render {

class CLightProcesser : public CProcesser
{
public:
	CLightProcesser(IMRender * Render);
	~CLightProcesser(void);

protected:
	// set rendersate , constants,shaders,effect
	virtual int Prepare(RenderState * State,RenderCommand * Command);
	// process a renderable
	virtual int ProcessRenderable(Renderable * pRenderable,int * ContextKey);
};

} // end namespce

#endif
