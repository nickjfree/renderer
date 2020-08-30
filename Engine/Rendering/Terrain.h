#ifndef __TERRAIN__
#define __TERRAIN__

#include  "TerrainNode.h"
#include "../Scene/Component.h"
#include "../Container/Vector.h"
#include "RenderObject.h"


constexpr int TerrainTileSize = 64;
constexpr int TerrainLevels = 8;
constexpr int VirtualTexturePageSize = 2048;  // 2K per page
constexpr int VirtualTexturePageScale = 6;
constexpr int VirtualTextureSize = VirtualTexturePageScale * VirtualTexturePageSize;
constexpr int VirtaulTexturePageTableSize = 4 * (128 * 128 + 64 * 64 + 32 * 32 + 16 * 16 + 8 * 8 + 4 * 4 + 2 * 2 + 1);


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
	// onattach
	int OnAttach(GameObject* GameObj);
	// destroy
	int OnDestroy(GameObject* GameObj);

};

#endif
