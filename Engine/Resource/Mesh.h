#ifndef __MESH__
#define __MESH__

#include "Resource.h"
/*
	the mesh resource
*/
class Mesh : public Resource
{
	DECLAR_ALLOCATER(Mesh);
	OBJECT(Mesh);
	BASEOBJECT(Resource);
public:
	Mesh(Context* context);
	virtual ~Mesh();
};

#endif
