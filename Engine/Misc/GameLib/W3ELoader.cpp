#include "StdAfx.h"
#include "W3ELoader.h"
#include "Terrain.h"


using TerrainSystem::W3E::CW3ELoader;
using TerrainSystem::W3E::W3EMapHeader;
using TerrainSystem::W3E::W3ETilePoint;
using TerrainSystem::W3E::W3ETileSetID;

CW3ELoader::CW3ELoader(void)
{
}

CW3ELoader::~CW3ELoader(void)
{
	delete m_MapHeader->ATileSet;
	delete m_MapHeader->BTileSet;
	delete m_MapHeader;
	delete m_TilePoint;
}

int CW3ELoader::LoadMap(char * FileName)
{
	HANDLE hFile;
	DWORD  Read;
	int Skip = 0;
	hFile = CreateFileA(FileName,GENERIC_READ,0,NULL,OPEN_EXISTING,NULL,NULL);
	m_MapHeader = new W3EMapHeader;
	//Read tileset first
	ReadFile(hFile,m_MapHeader,17,&Read,NULL);
	m_MapHeader->ATileSet = new W3ETileSetID[m_MapHeader->NumATileSet];
	ReadFile(hFile,m_MapHeader->ATileSet,m_MapHeader->NumATileSet * 4,&Read,NULL);
	ReadFile(hFile,&m_MapHeader->NumBTileSet,4,&Read,NULL);
	m_MapHeader->BTileSet = new W3ETileSetID[m_MapHeader->NumBTileSet];
	ReadFile(hFile,m_MapHeader->BTileSet,m_MapHeader->NumBTileSet * 4,&Read,NULL);
	//Read rest
	ReadFile(hFile,&m_MapHeader->PointWidth,16,&Read,NULL);

	// Record some data
	m_Width = m_MapHeader->PointWidth;
	m_Height = m_MapHeader->PointHeight;

	//Read TilePoints
	m_TilePoint = new W3ETilePoint[m_Width*m_Height];

	ReadFile(hFile,m_TilePoint,m_Width*m_Height*sizeof(W3ETilePoint),&Read,NULL);
	//it's done
	return 0;
}

int TerrainSystem::W3E::CW3ELoader::Width(void)
{
	return m_Width;
}

int TerrainSystem::W3E::CW3ELoader::Height(void)
{
	return m_Height;
}

int TerrainSystem::W3E::CW3ELoader::GetTilePoint(TilePoint * Point)
{
	for(int i = 0;i < m_Height * m_Width;i++)
	{
		Point[i].Layer = m_TilePoint[i].CliffTextureTypeAndLayerHeight & 0x0F;
		Point[i].GroundType = m_TilePoint[i].FlagAndTexturType & 0x0F;
		//ground_height - 0x2000 + (layer - 2)*0x0200
		Point[i].Height = (m_TilePoint[i].GroundHeight - 0x2000 + (Point[i].Layer - 2) * 0x200)/4.0f;
		Point[i].Height = Point[i].Height/16.0f;
		Point[i].Flag = m_TilePoint[i].FlagAndTexturType & 0x0F;
	}
	return 0;
}
