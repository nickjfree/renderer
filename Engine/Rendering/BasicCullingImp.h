#ifndef __BASIC_CULLING__
#define __BASIC_CULLING__

#include "Spatial.h"


/*
	Basic culling method
*/
class BasicCullingImp : public Spatial
{
public:
	BasicCullingImp();
	~BasicCullingImp();
	// insert
	virtual int Add(RenderObject* RenderObj);
	//remove
	virtual int Remove(RenderObject* RenderObj);
	// query by frustum
	virtual int Query(Frustum& Fr, Vector<Node*>& Result, int Type);
	// query by aabb
	virtual int Query(AABB& Box, Vector<Node*>& Result, int Type);
	// query by type
	virtual int Query(Vector<Node*>& Result, int Type);
};


#endif


