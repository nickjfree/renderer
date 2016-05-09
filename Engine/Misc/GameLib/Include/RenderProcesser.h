#ifndef __RENDER_PROCESSER__
#define __RENDER_PROCESSER__


#include "IMRender.h"
#include "RenderQueue.h"


namespace Render {


class CProcesser
{
protected:
	IMRender *		  m_Render;
	RenderState *     m_RenderState;
	int               m_LightIndex;
public:
	CProcesser(IMRender * Render);
	virtual ~CProcesser(void);
	// call processRenderable in this function
	int ProcessCommand(RenderState * State,RenderCommand * Command);
	int SetLightIndex(int Index) {m_LightIndex = Index; return 0;};
	int SetRenderState(RenderState * State) {m_RenderState = State; return 0;};
protected:
	// set rendersate , constants,shaders,effect
	virtual int Prepare(RenderState * State,RenderCommand * Command);
	// process a renderable
	virtual int ProcessRenderable(Renderable * pRenderable,int * ContextKey);
};

} // end namespace


#endif
