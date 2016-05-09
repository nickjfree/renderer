#pragma once
#include "TilePoint.h"

using TerrainSystem::TilePoint;

namespace TerrainSystem { namespace W3E{

#pragma pack(1)

typedef struct _W3ETileSetID
{
	char ID[4];
}W3ETileSetID;

typedef struct _W3EMapHeader
{
	char Magic[4];
    int  Version;
    char MainTileset;
    int  UseCustomTile;
    int  NumATileSet;
    W3ETileSetID * ATileSet;
    int  NumBTileSet;
    W3ETileSetID * BTileSet;
    int  PointWidth;
    int  PointHeight;
    float CenterOffsetX;
    float CenterOffsetY;    
}W3EMapHeader;

typedef struct _W3ETilePoint
{
    short GroundHeight;
    short WaterLevelAndEdgeBound;
    unsigned char  FlagAndTexturType;
    unsigned char  TextureDetail;
    unsigned char  CliffTextureTypeAndLayerHeight;
}W3ETilePoint;

#pragma pack()


class CW3ELoader
{
public:
	CW3ELoader(void);
	~CW3ELoader(void);

private:
	W3ETilePoint * m_TilePoint;
	W3EMapHeader * m_MapHeader;

	//width and height
	int m_Width;
	int m_Height;
private:

public:
	int LoadMap(char * FileName);

	int Width(void);
	int Height(void);
	int GetTilePoint(TilePoint * Point);
};

}}//end namespace