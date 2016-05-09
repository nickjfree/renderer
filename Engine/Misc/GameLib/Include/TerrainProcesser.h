#ifndef __TERRAIN_PROCESSER__
#define __TERRAIN_PROCESSER__

#include "NormalSolidProcesser.h"

namespace Render {

class CTerrainProcesser :
	public CNormalSolidProcesser
{
public:
	CTerrainProcesser(IMRender * Render);
	~CTerrainProcesser(void);
protected:
	virtual int ProcessRenderable(Renderable * pRenderable);
};




class CLPPTerrainProcesser : public CTerrainProcesser
{
public:
	CLPPTerrainProcesser(IMRender * Render);
	~CLPPTerrainProcesser(void);
protected:
	virtual int Prepare(RenderState * State,RenderCommand * Command);
};

}//end namespace


#endif
