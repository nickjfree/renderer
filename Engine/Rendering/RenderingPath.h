#ifndef __RENDERING_PATH__
#define __RENDERING_PATH__


#include "RenderStage.h"
#include "Container\Vector.h"

/*
	Rendering path
*/

class RenderingPath
{
public:
	// stages
	Vector<RenderStage*> Stages;
public:
	RenderingPath(RenderContext * Context);
	virtual ~RenderingPath();
	int Execute();
};

#endif