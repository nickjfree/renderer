#ifndef __NORMALSOLID_P__
#define __NORMALSOLID_P__

#include "DefaultProcesser.h"

namespace Render {

class CNormalSolidProcesser : public CDefaultProcesser
{
public:
	CNormalSolidProcesser(IMRender * Render);
	~CNormalSolidProcesser(void);

protected:
	// set rendersate , constants,shaders,effect
	virtual int Prepare(RenderState * State,RenderCommand * Command);
	// process a renderable
	virtual int ProcessRenderable(Renderable * pRenderable);

};

class CLPPNormalSolidProcesser : public CNormalSolidProcesser
{
public:
	CLPPNormalSolidProcesser(IMRender * Render);
	~CLPPNormalSolidProcesser(void);

protected:
	// set rendersate , constants,shaders,effect
	virtual int Prepare(RenderState * State,RenderCommand * Command);

	virtual int ProcessRenderable(Renderable * pRenderable);
};

} //end namespace

#endif
