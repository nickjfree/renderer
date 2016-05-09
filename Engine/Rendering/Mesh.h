#ifndef __MESH__
#define __MESH__

#include "GPUResource.h"
#include "Math\Collision.h"
#include "Math\LinearMath.h"

/*
	the mesh resource
*/
class Mesh : public GPUResource
{
	DECLAR_ALLOCATER(Mesh);
	OBJECT(Mesh);
protected: 
	AABB Box;
public:
	Mesh(Context* context);
	virtual ~Mesh();
	AABB& GetAABB() { return Box; };
};

#endif
