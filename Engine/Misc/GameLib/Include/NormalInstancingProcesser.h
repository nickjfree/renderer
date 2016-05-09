#ifndef __NORMAL_INSTANCING_PROCESSER__
#define __NORMAL_INSTANCING_PROCESSER__

#include "RenderProcesser.h"

namespace Render {

class CNormalInstancingProcesser : public CProcesser
{
public:
	CNormalInstancingProcesser(IMRender * Render);
	~CNormalInstancingProcesser(void);
protected:
	int Prepare(RenderState * State,RenderCommand * Command);
	int ProcessRenderable(Renderable * pRenderable);
};



class CLPPNormalInstancingProcesser : public CNormalInstancingProcesser
{
public:
	CLPPNormalInstancingProcesser(IMRender * Render);
	~CLPPNormalInstancingProcesser(void);

protected:
	int Prepare(RenderState * State,RenderCommand * Command);
	
	int ProcessRenderable(Renderable * pRenderable);
};

}//end namespace

#endif
