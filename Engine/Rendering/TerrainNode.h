#ifndef __TERRAIN_NODE__
#define __TERRAIN_NODE__

#include "RenderObject.h"
#include "TerrainPayload.h"

class Terrain;

class TerrainNode : public RenderObject, Resource
{
	OBJECT(TerrainNode);
	BASEOBJECT(TerrainNode);
	DECLARE_ALLOCATER(TerrainNode)
	LOADEROBJECT(FileLoader);

private:
	// Terrain
	Terrain* terrain_;
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
	// view distance
	float viewDistance_;
private:
	// isVisible
	bool IsVisible(const Vector3& viewPoint);
public:
	TerrainNode(Context* context);
	// on sub resource create complete(main thread)
	virtual int OnSubResource(int Message, Resource* Sub, Variant& Param);
	// set scale
	void SetScale(int scale) { scale_ = scale; }
	// set terrain
	void SetTerrain(Terrain* terrain) { terrain_ = terrain; }
	// get terrain
	Terrain* GetTerrain() { return terrain_; }
	// set level
	void SetLevel(int level);
	// query
	virtual int Query(Frustum& Fr, Vector<Node*>& Result, int Types, bool inside);
	// compile
	virtual int Compile(BatchCompiler* Compiler, int Stage, int Lod, Dict& StageParameter, RenderingCamera* Camera, RenderContext* Context);
};



#endif
