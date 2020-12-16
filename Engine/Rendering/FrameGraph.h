#ifndef __FRAME_GRAPH__
#define __FRAME_GRAPH__

#include "Container/Dict.h"





/*
*  frame graph implementation
*/


/*
	render pass
*/

class RenderingPass
{

};

/*
	renderingqueue
*/
class RenderingQueue
{

};


/*
	frame graph
*/
class FrameGraph
{
private:
	// render passes
	Vector<RenderingPass> renderPasses;
};

#endif

