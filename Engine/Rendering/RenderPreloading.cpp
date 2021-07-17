#include "RenderPreloading.h"


USING_ALLOCATER(RenderPreloading)

RenderPreloading::RenderPreloading(Context* context) : GPUResource(context)
{
}


RenderPreloading::~RenderPreloading()
{
}

int RenderPreloading::OnSubResource(int Message, Resource* Sub, Variant& Param) {
	String name = *Param.as<String*>();
	Variant resource;
	resource= Sub;
	rendercontext->SetResource(name, resource);
	return 0;
}