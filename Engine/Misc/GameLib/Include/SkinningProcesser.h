#ifndef __SKINNING_PROCESSER__
#define __SKINNING_PROCESSER__

#include "NormalSolidProcesser.h"

namespace Render {

class CSkinningProcesser :
	public CNormalSolidProcesser
{
public:
	CSkinningProcesser(IMRender * Render);
	~CSkinningProcesser(void);

protected:
	int ProcessRenderable(Renderable * pRenderable);
};



class CLPPSkinningProcesser : public CSkinningProcesser
{
public:
	CLPPSkinningProcesser(IMRender * Render);
	~CLPPSkinningProcesser(void);

protected:
	int virtual Prepare(RenderState * State,RenderCommand * Command);
	int virtual ProcessRenderable(Renderable * pRenderable);
};

}// end namespace


#endif
