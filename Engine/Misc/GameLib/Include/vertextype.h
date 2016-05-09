#ifndef _VERTEX_TYPE_
#define _VERTEX_TYPE_

#include "IMRender.h"

#define VTYPE_DYNAMIC_INSTANCING  0
#define VTYPE_TERRAIN             1
#define VTYPE_SOLID               2
#define VTYPE_SKINNING           11
#define VTYPE_LIGHTING            7



#define VTYPE_STATIC_INSTANCING   8
#define VTYPE_LIGHT_INSTANCING    9
#define VTYPE_STATIC_BONE_ID      0

#pragma pack(1)


//vertex type
//dynamic_instancing
typedef struct _vertex_dynamic_instancing
{
	Vector3 position;
	Vector3 normal;
	float u,v;
	float instance_id;              // ID and TeamInfo or BoneID
	Vector3 tangent;
}vertex_dynamic_instancing;

typedef vertex_dynamic_instancing vertex_static_bone_id;

// static instanceing   sm 3.0  d3d10 sm 4.0
typedef struct _vertex_static_instancing       // instancing for normal object
{
	float       progress;
	Matrix4x4   TransMatrix;
}vertex_static_instancing;

//terrain rendering with instancing. LOD levels
typedef struct _vertext_terrain_instancing
{
	float    scale;
	Vector3  position;
}vertex_terrain_instancing;

typedef struct _vertex_light_instancing  // instancing for light objects
{
	float      c0,c1,c2,c3;           // color
	Vector3    position;           // position, light center
}vertex_light_instancing;


typedef struct _vertex_terrain
{
	Vector3    position;
	Vector3    normal;
	float      u,v;
	float      a0,a1,a2,a3;
	Vector3    tangent;
}vertex_terrain;

#pragma pack()






#endif