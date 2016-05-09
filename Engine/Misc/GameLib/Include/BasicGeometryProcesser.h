#ifndef __BASIC_GEOMETRY_PROCESSER__
#define __BASIC_GEOMETRY_PROCESSER__

#include "NormalInstancingProcesser.h"

namespace Render {

class CBasicGeometryProcesser :
	public CNormalInstancingProcesser
{
public:
	CBasicGeometryProcesser(IMRender * Render);
	~CBasicGeometryProcesser(void);

protected:
	int Prepare(RenderState * State,RenderCommand * Command);

};

}//end namespace

#endif
