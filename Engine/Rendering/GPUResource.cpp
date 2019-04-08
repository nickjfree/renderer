#include "GPUResource.h"
#include "RenderingSystem.h"


GPUResource::GPUResource(Context * context) : Resource(context)
{
    // do the init stuff, just set some pointers
    RenderingSystem * render = context->GetSubsystem<RenderingSystem>();
    renderinterface = render->GetRenderInterface();
    rendercontext = render->GetRenderContext();
}


GPUResource::~GPUResource()
{
}
