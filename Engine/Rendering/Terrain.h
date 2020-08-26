#ifndef __TERRAIN__
#define __TERRAIN__

#include  "TerrainNode.h"
#include "../Scene/Component.h"
#include "../Container/Vector.h"
#include "RenderObject.h"


constexpr int TerrainTileSize = 32;
constexpr int TerrainLevels = 8;


class Terrain : public Component
{
	OBJECT(Terrain);
	BASEOBJECT(Terrain);
	DECLARE_ALLOCATER(Terrain);

private:
	// root nodes of quad tree
	Vector<TerrainNode*> rootNodes;
	// all the nodes
	Vector<TerrainNode*> Nodes_;
	// the world size
	int sizeX_, sizeY_;
	// clipmap mesh 
	int clipmapGeometry_;
private:
	// create node
	TerrainNode * CreateNode(int x, int y, int level);

public:
	Terrain(Context* context);
	// init terrain 
	void Init(int sizeX, int sizeY, int numLevels);


};

#endif
