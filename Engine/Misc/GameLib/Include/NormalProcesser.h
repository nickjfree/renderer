#ifndef __NORMAL_PROCESSER__
#define __NORMAL_PROCESSER__


#include "renderprocesser.h"

namespace Render {

class CNormalProcesser : public CProcesser
{
public:
	CNormalProcesser(IMRender * Render);
	~CNormalProcesser(void);
protected:
	// set rendersate , constants,shaders,effect
	virtual int Prepare(RenderState * State,RenderCommand * Command);
	// process a renderable
	virtual int ProcessRenderable(Renderable * pRenderable,int * ContextKey);
};

} // end namespace

#endif
