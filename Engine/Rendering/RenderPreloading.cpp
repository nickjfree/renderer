#include "RenderPreloading.h"



RenderPreloading::RenderPreloading(Context * context) : GPUResource(context)
{
}


RenderPreloading::~RenderPreloading()
{
}

int RenderPreloading::OnSubResource(int Message, Resource * Sub, Variant& Param) {
	String name = *Param.as<String*>();
	Variant resource;
	resource.as<Resource*>() = Sub;
	rendercontext->SetResource(name, resource);
	return 0;
}