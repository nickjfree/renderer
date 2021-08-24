#include "GIVolume.h"



GIVolume::GIVolume()
{
	Type = Node::GIVOLUME;
}


int GIVolume::Render(CommandBuffer* cmdBuffer, int stage, int lod, RenderingCamera* camera, RenderContext* renderContext)
{
	return 0;
}
