#ifndef __TILEPOINT__
#define __TILEPOINT__

#define  TILESIZE 4

//corner
#define  UP_LEFT   0
#define  UP_RIGHT  1
#define  LOW_LEFT  2
#define  LOW_RIGHT 3
 
namespace TerrainSystem{

typedef struct _TilePoint
{
	BYTE  GroundType;
	float Height;
	short Layer;
	BYTE  Flag;
}TilePoint;


} //end namespace
#endif