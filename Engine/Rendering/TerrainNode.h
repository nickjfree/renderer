#ifndef __TERRAIN_NODE__
#define __TERRAIN_NODE__

#include "Node.h"
#include "TerrainPayload.h"


class TerrainNode : public Node, Resource
{
	OBJECT(TerrainNode);
	BASEOBJECT(TerrainNode);
	DECLARE_ALLOCATER(TerrainNode)
	LOADEROBJECT(FileLoader);

private:
	// terrain payload (Terrain Data for rendering)
	TerrainPayload* terrainPayload_;
	// terrain data status
	enum class PayloadState {
		PAGED_OUT = 0,
		PAGED_IN = 1
	};
	// payload state
	PayloadState payloadState_;
	// terrain size. height and width
	int scale_;
	// LOD
	int level_;
	// position offset
	int offsetX_, offsetY_;
private:

public:
	TerrainNode(Context* context);
	// on sub resource create complete(main thread)
	virtual int OnSubResource(int Message, Resource* Sub, Variant& Param);
	// set scale
	void SetScale(int scale) { scale_ = scale; }
};



#endif
