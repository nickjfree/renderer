#ifndef __H3D_H__
#define __H3D_H__

#define H3DMAGIC   "HU3D"


namespace h3d {

#pragma pack(1)



typedef struct h3d_mesh
{
	DWORD VertexSize;
	DWORD IndexSize;
	DWORD VertexNum;
	DWORD OffsetVertex;
	DWORD IndexNum;
	DWORD OffsetIndex;
	char  Texture[256];
}h3d_mesh;

//280

typedef struct h3d_bone
{
	int    Frames;
	int    BoneNum;
	DWORD  OffsetBone;
}h3d_bone;

//12

typedef struct h3d_header
{
	DWORD  Magic;
	DWORD  Version;
	int    MeshNum;
}h3d_header;

typedef struct h3d_vertex
{
	float x, y, z;
	float nx, ny, nz;
	float u, v;
	float parameter;
	float tx, ty, tz, tw;
}h3d_vertex;

//12

#pragma pack()

} //end namespace

#endif