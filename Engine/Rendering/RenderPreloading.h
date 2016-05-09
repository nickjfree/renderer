#ifndef __RENDER_PRE_LOADING__
#define __RENDER_PRE_LOADING__


#include "GPUResource.h"


class RenderPreloading : public GPUResource {

public:
	RenderPreloading(Context * context);
	virtual ~RenderPreloading();
	virtual int OnSubResource(int Message, Resource * Sub, Variant& Param);
};

#endif

