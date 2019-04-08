#ifndef __GPU_RESOURCE__
#define __GPU_RESOURCE__

#include "Resource\Resource.h"
//#include "RenderingSystem.h"
#include "RenderInterface.h"
#include "RenderContext.h"

/*
    CPU resource base
*/
class GPUResource : public Resource
{
    OBJECT(GPUResource);
    BASEOBJECT(Resource);
protected:
    // render api interface
    RenderInterface * renderinterface;
    // render context
    RenderContext * rendercontext;
    // resource id in GPU
    int id;
public:
    GPUResource(Context * context);
    ~GPUResource();
    int GetId() { return id; }
};

#endif
