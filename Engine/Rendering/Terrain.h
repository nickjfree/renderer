#ifndef __TERRAIN__
#define __TERRAIN__

#include  "TerrainNode.h"
#include "../Scene/Component.h"
#include "../Container/Vector.h"
#include "RenderObject.h"

class Terrain : public Component
{
	OBJECT(Terrain);
	BASEOBJECT(Terrain);
	DECLARE_ALLOCATER(Terrain);

private:
	// root nodes of quad tree
	Vector<TerrainNode*> rootNodes;
	// the world size
	int sizeX_, sizeY_;

public:
	Terrain(Context* context);
	// init terrain 
	void Init(int sizeX, int sizeY, int level);


};

#endif
