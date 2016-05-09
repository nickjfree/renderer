#include "Include\Terrain.h"
#include "StdAfx.h"
#include "GameLib.h"
#include "Include\Terrain.h"
#include "Include\GameCamera.h"
#include "ResourceManager.h"
#include "RenderEngine.h"


using TerrainSystem::CTerrain;
using TerrainSystem::CPointMap;
using TerrainSystem::CTile;
//using TerrainSystem::TilePoint;
using TerrainSystem::W3E::CW3ELoader;
using Render::CRender;
using Render::CRenderQueue;
//using resource manager
using ResourceManager::CResourceManager;
using namespace Render;

CTerrain::CTerrain()
{
}

CTerrain::~CTerrain(void)
{
}



CTile::CTile(void)
{
	m_TileWidth = 10;
	strcpy(m_Type,"TerrainTile");
}

CTile::~CTile(void)
{
}

CPointMap::CPointMap(void)
{
}

CPointMap::~CPointMap(void)
{
}

int CPointMap::LoadTilePoint(char * FileName)
{
	m_W3ELoader.LoadMap(FileName);
	m_WidthOfPointMap = m_W3ELoader.Width();
	m_HeightOfPointMap = m_W3ELoader.Height();
	// Convert
	m_TilePoint = new TilePoint[m_WidthOfPointMap * m_HeightOfPointMap];
	m_W3ELoader.GetTilePoint(m_TilePoint);
	return 0;
}

float CPointMap::GetHeight(int x, int y)
{
	return 0;
}

int CPointMap::GetLayer(int x, int y)
{
	return 0;
}

int CPointMap::GetGroundType(int x, int y)
{
	return 0;
}

int CTile::Create(TilePoint ** Corner,WORD * SharedIndex,int x,int y)
{
	m_LocationX = x;
	m_LocationY = y;
	CreateCorner(Corner);
	if(IsNormalGround())
	{
		//..........
		m_SharedIndex = SharedIndex;
		CreateSameLayer();
	}
	m_BoundRect = CRect2D(m_LocationX * m_TileWidth,m_LocationY * m_TileWidth,
		(m_LocationX+1) * m_TileWidth,(m_LocationY+1) * m_TileWidth);
	return 0;
}
;
int CTile::CreateCorner(TilePoint ** Corner)
{
	m_Corner[UP_LEFT] = *Corner[UP_LEFT];
	m_Corner[UP_RIGHT] = *Corner[UP_RIGHT];
	m_Corner[LOW_RIGHT] = *Corner[LOW_RIGHT];
	m_Corner[LOW_LEFT] = *Corner[LOW_LEFT];
	return 0;
}

int CTile::CreateSameLayer(void)
{
	CreateSameLayerVertex();
	CreateSameLayerUV();
	CreateSameLayerNormal();
	CreateGroundTypeAlpha();
	CreateSameLayerIndex();
	return 0;
}

int CTile::CreateSameLayerIndex(void)
{
	m_Index = m_SharedIndex;
	m_NumIndex = 6;
	return 0;
}

int CTile::CreateSameLayerVertex(void)
{
	Vector3 tangent = Vector3(1,0,0);
	m_NormalGroundVertex[UP_LEFT].position._x = m_LocationX * m_TileWidth;
	m_NormalGroundVertex[UP_LEFT].position._z = m_LocationY * m_TileWidth;
	m_NormalGroundVertex[UP_LEFT].position._y = 2;//m_Corner[UP_LEFT].Height;
	m_NormalGroundVertex[UP_LEFT].tangent = tangent;
	m_NormalGroundVertex[UP_RIGHT].position._x = (m_LocationX + 1) * m_TileWidth;
	m_NormalGroundVertex[UP_RIGHT].position._z = m_LocationY * m_TileWidth;
	m_NormalGroundVertex[UP_RIGHT].position._y = 2;//m_Corner[UP_RIGHT;].Height;
	m_NormalGroundVertex[UP_RIGHT].tangent = tangent;
	m_NormalGroundVertex[LOW_LEFT].position._x = m_LocationX * m_TileWidth;
	m_NormalGroundVertex[LOW_LEFT].position._z = (m_LocationY +1)* m_TileWidth;
	m_NormalGroundVertex[LOW_LEFT].position._y = 2;//m_Corner[LOW_LEFT].Height;
	m_NormalGroundVertex[LOW_LEFT].tangent = tangent;
	m_NormalGroundVertex[LOW_RIGHT].position._x = (m_LocationX + 1) * m_TileWidth;
	m_NormalGroundVertex[LOW_RIGHT].position._z = (m_LocationY + 1) * m_TileWidth;																																									
	m_NormalGroundVertex[LOW_RIGHT].position._y = 2;//m_Corner[LOW_RIGHT].Height;
	m_NormalGroundVertex[LOW_RIGHT].tangent = tangent;
	m_Vertex = m_NormalGroundVertex;
	m_NumVertex = 4;
	return 0;
}

int CTile::SetTexture(int SkinID)
{
	m_SkinID = SkinID;
	return 0;
}

int CTerrain::LoadW3E(char * FileName)
{
	m_PointMap.LoadTilePoint(FileName);
	m_MapWidth = m_PointMap.Width() - 1;
	m_MapHeight = m_PointMap.Height() - 1;
	return 0;
}

TilePoint * CPointMap::GetPoint(int x, int y)
{
	return &m_TilePoint[y * m_WidthOfPointMap + x];
}

int CTerrain::Create(void)
{
	//Create Tiles
	//Index shared by nromal tiles
	m_SharedIndex = new WORD[6];
	m_SharedIndex[0] = 0;
	m_SharedIndex[1] = 1;
	m_SharedIndex[2] = 2;
	m_SharedIndex[3] = 2;
	m_SharedIndex[4] = 1;
	m_SharedIndex[5] = 3;
	//Init tiles
	m_TileMaterial = CRenderEngine::GetRenderEngine()->GetMaterialManager()->CreateMaterial("terrain");
	m_Tiles = new CTile[m_MapWidth * m_MapHeight];
	for(int i = 0;i < m_MapHeight;i++)
	{
		for(int j = 0;j < m_MapWidth;j++)
		{
			TilePoint * Corner[4];
			Corner[UP_LEFT] = m_PointMap.GetPoint(j,i);
			Corner[UP_RIGHT] = m_PointMap.GetPoint(j + 1,i);
			Corner[LOW_LEFT] = m_PointMap.GetPoint(j,i + 1);
			Corner[LOW_RIGHT] = m_PointMap.GetPoint(j + 1,i + 1);
			m_Tiles[i * m_MapHeight + j].Create(Corner,m_SharedIndex,j,i);
			m_Tiles[i * m_MapHeight + j].SetTexture(m_Texture);
			m_Tiles[i * m_MapHeight + j].SetMaterial(m_TileMaterial,0);
		}
	}
	return 0;
} 

int CPointMap::Width(void)
{
	return m_WidthOfPointMap;
}

int CPointMap::Height(void)
{
	return m_HeightOfPointMap;
}

bool CTile::IsNormalGround(void)
{
	return m_Corner[0].Layer == m_Corner[1].Layer && m_Corner[0].Layer == m_Corner[2].Layer && m_Corner[0].Layer == m_Corner[3].Layer;
}

int CTile::CreateSameLayerNormal(void)
{
	return 0;
}

int CTile::CreateGroundTypeAlpha(void)
{
	for(int i = UP_LEFT;i <= LOW_RIGHT;i++)
	{
		m_NormalGroundVertex[i].a0 = 0;
		m_NormalGroundVertex[i].a1 = 0;
		m_NormalGroundVertex[i].a2 = 0;
		m_NormalGroundVertex[i].a3 = 0;
		if(m_Corner[i].GroundType)
		{
			m_NormalGroundVertex[i].a1 = 1;

		}
		else
		{
			m_NormalGroundVertex[i].a0 = 1;
		}
	}
	// smoth
	return 0;
}

int CTile::Render(int LOD)
{
	CRender * Render = CRender::GetRender();
	Render->AddRenderable(NULL,m_Material[LOD],this);
	CSceneNode::Render(LOD);
	return 0;
}

int CTile::RenderLightPrePass()
{
	return 0;
}


int CTile::CreateSameLayerUV(void)
{
	m_NormalGroundVertex[UP_LEFT].u = 0;
	m_NormalGroundVertex[UP_LEFT].v = 0;
	m_NormalGroundVertex[UP_RIGHT].u = 1;
	m_NormalGroundVertex[UP_RIGHT].v = 0;
	m_NormalGroundVertex[LOW_LEFT].u = 0;
	m_NormalGroundVertex[LOW_LEFT].v = 1;
	m_NormalGroundVertex[LOW_RIGHT].u = 1;
	m_NormalGroundVertex[LOW_RIGHT].v = 1;
	return 0;
}

int CTerrain::SetTexture(int TileSet)
{
	wchar_t * List[8] = {0,0,0,0,0,0,0,0};
	List[0] = L"dirt.dds";
	List[1] = L"grass.dds";
	COLOR color = COLOR(1,1,1,1);
//	m_Texture = m_Render->CreateSkin(&color,&color,&color,&color,0,List);       // should create from resourcemanager
//	CResourceManager * ResourceManager = CResourceManager::GetResourceManager();
//	m_Texture = ResourceManager->RegisterTexture(List);
	//ResourceManager->FlushCreationCMD();
	return 0;
}

CRect2D& TerrainSystem::CTile::GetBoundRect2D(void)
{
	return m_BoundRect;
}

int TerrainSystem::CTerrain::ToSceneManager(CQuadTree * QuadTree)
{
	CRect2D WorldRect(0,0,m_MapWidth * 10.0f,m_MapHeight * 10.0f);
	QuadTree->Construct(WorldRect,40);
	for(int i = 0;i < m_MapWidth * m_MapHeight;i++)
	{
		QuadTree->AddSenceNode(&m_Tiles[i]);
	}
	return 0;
}


int TerrainSystem::CTile::DrawNormalPass(IMRender * Render,int * ContextKey,void * RenderParameter)
{
	Render->Render(0,m_Vertex,m_NumVertex*sizeof(vertex_terrain),m_Index,m_NumIndex);
	return 0;
}

int TerrainSystem::CTile::DrawLightPrePass(IMRender * Render,int * ContextKey,void * RenderParameter)
{
	DrawNormalPass(Render,ContextKey,RenderParameter);
	return 0;
}
