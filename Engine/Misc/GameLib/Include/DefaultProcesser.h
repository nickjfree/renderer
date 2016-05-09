#ifndef __DEFAULT_P__
#define __DEFAULT_P__

#include "RenderProcesser.h"

namespace Render {

class CDefaultProcesser : public CProcesser
{
public:
	CDefaultProcesser(IMRender * Render);
	~CDefaultProcesser(void);
};

}

#endif
