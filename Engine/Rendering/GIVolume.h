#ifndef __GI_VOLUME__
#define __GI_VOLUME__


#include "Node.h"



class GIVolume: public Node
{
public:
	GIVolume();
	// queue render command
	virtual int Render(CommandBuffer* cmdBuffer, int stage, int lod, RenderingCamera* camera, RenderContext* renderContext);
	
private:
	// volume describe
	CBGIVolume giVolume;
};


#endif

