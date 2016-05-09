#ifndef __SPATIAL__
#define __SPATIAL__

#include "Math\LinearMath.h"
#include "Math\Collision.h"
#include "Node.h"
#include "RenderObject.h"
#include "Container\Vector.h"

#include "Container\RecyclePool.h"

/*
	render object base spatial class
*/

class Spatial
{
protected:
	Node * Root;
public:
	// insert
	virtual int Add(RenderObject* RenderObj);
	//remove
	virtual int Remove(RenderObject* RenderObj);
	// query by frustum
	virtual int Query(Frustum& Fr, Vector<Node*>& Result, int Type);
	// query by aabb
	virtual int Query(AABB& Box, Vector<Node*>& Result, int Type);
public:
	Spatial();
	~Spatial();
};

#endif