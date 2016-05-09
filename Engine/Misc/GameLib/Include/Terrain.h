#ifndef __TERRAIN_H__
#define __TERRAIN_H__

#include "IMRender.h"
#include "SceneNode.h"
#include "vertextype.h"
#include "w3eloader.h"
#include "TilePoint.h"
#include "Render.h"
#include "ResourceManager.h"
#include "QuadTree.h"





using TerrainSystem::W3E::CW3ELoader;
using SceneManager::CSceneNode;
using SceneManager::CRect2D;
using Render::CRender;
using SceneManager::CQuadTree;

namespace TerrainSystem{

class CTerrain;

class CPointMap
{
public:
	CPointMap(void);
	~CPointMap(void);
private:
	int         m_WidthOfPointMap;
	int         m_HeightOfPointMap;
	TilePoint * m_TilePoint;
	//loaders
	CW3ELoader  m_W3ELoader;
public:
	int LoadTilePoint(char * FileName);
	float GetHeight(int x, int y);
	int GetLayer(int x, int y);
	int GetGroundType(int x, int y);
	TilePoint * GetPoint(int x, int y);
	int Width(void);
	int Height(void);
};

class CTile : public CSceneNode
{
public:
	CTile();
	~CTile();
private:
	//  Geometry Data
	int              m_ID;
	vertex_terrain   m_NormalGroundVertex[4];
	vertex_terrain * m_Vertex;
	WORD *           m_Index;
	DWORD            m_NumVertex;
	DWORD            m_NumIndex;
	// Status
	DWORD            m_TileType;
	int              m_SkinID;
	//attribute
	int              m_LocationX;
	int              m_LocationY;
	float            m_TileWidth;
	TilePoint        m_Corner[4];
	//father
	CTerrain *       m_Terrain;
	//shared data
	WORD *    m_SharedIndex;
	//BoundRect
	CRect2D m_BoundRect;        // 2D BoundRect
public:
	int Create(TilePoint ** Corner,WORD * SharedIndex,int x,int y);
private:
	int CreateCorner(TilePoint ** Corner);
	int CreateSameLayer(void);
private:
	int CreateSameLayerIndex(void);
	int CreateSameLayerVertex(void);
public:
	bool IsNormalGround(void);
	int CreateSameLayerNormal(void);
	int CreateGroundTypeAlpha(void);
	int virtual Render(int LOD);
	int virtual RenderLightPrePass();
	int CreateSameLayerUV(void);
	CRect2D& GetBoundRect2D(void);
	int SetTexture(int SkinID);
	virtual int DrawNormalPass(IMRender * Render,int * ContextKey,void * RenderParameter);
	virtual int DrawLightPrePass(IMRender * Render,int * ContextKey,void * RenderParameter);
};


class CTerrain
{
private:
	//data
	CTile *    m_Tiles;
	CPointMap  m_PointMap;
	short      m_Texture;
	//map attributs
	int m_MapWidth;
	int m_MapHeight;
	//shared index
	WORD * m_SharedIndex;
	CMaterial * m_TileMaterial;
public:
	CTerrain();
	virtual ~CTerrain(void);
	int LoadW3E(char * FileName);
	int Create(void);
	int SetTexture(int TileSet);
	int ToSceneManager(CQuadTree * Tree);
};

} // end namespace


#endif