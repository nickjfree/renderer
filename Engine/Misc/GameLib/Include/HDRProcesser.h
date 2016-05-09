#ifndef __HDR_PROCESSER__
#define __HDR_PROCESSER__


#include "renderprocesser.h"



namespace Render {

class CHDRProcesser : public CProcesser
{
public:
	CHDRProcesser(IMRender * Render);
	~CHDRProcesser(void);

protected:
	// set rendersate , constants,shaders,effect
	virtual int Prepare(RenderState * State,RenderCommand * Command);
	// process a renderable
	virtual int ProcessRenderable(Renderable * pRenderable,int * ContextKey);
};


}// end namespace

#endif
