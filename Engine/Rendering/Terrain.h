#ifndef __TERRAIN__
#define __TERRAIN__

#include  "TerrainNode.h"
#include "../Scene/Component.h"
#include "../Container/Vector.h"
#include "RenderObject.h"

typedef struct TerrainVertex {
	// position
	float x, y, z;
	// ring level
	float ringLevel;
	// sampleLevel
	float sampeLevel;
	// pad
	float id;
}TerrainVertex;


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
	// clipmap mesh 
	int clipmapGeometry_;
private:
	
public:
	Terrain(Context* context);
	// init terrain 
	void Init(int sizeX, int sizeY, int numLevels);


};

#endif
