#ifndef __POST_PROCESSER__
#define __POST_PROCESSER__


#include "renderprocesser.h"

namespace Render {


class CPostProcesser : public Render::CProcesser
{
public:
	CPostProcesser(IMRender * Render);
	~CPostProcesser(void);
protected:
	// set rendersate , constants,shaders,effect
	virtual int Prepare(RenderState * State,RenderCommand * Command);
	// process a renderable
	virtual int ProcessRenderable(Renderable * pRenderable,int * ContextKey);
};

}//end namespace



#endif
