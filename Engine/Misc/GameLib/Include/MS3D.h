#ifndef  _MS3D__
#define __MS3D__

#include "IMRender.h"
#include "VertexType.h"

namespace ModelSystem{namespace ms3d{

#pragma pack (1)

typedef struct _MS3D_VERTEX
{
	unsigned char flags;
	Vector3 vertex;
	char    Bone_ID;
	unsigned char  Unused;
}MS3D_VERTEX;

typedef struct _MS3D_TRIANGLE
{
	WORD flag;
	WORD index[3];
	Vector3 normal[3];
	float TextureCoorU[3];
	float TextureCoorV[3];
	BYTE SmoothGroup;
	BYTE Group_Index;
}MS3D_TRIANGLE;

typedef struct _MS3D_MESH
{
	BYTE flag;
	char Name[32];
	WORD NumTriangles;
	WORD * Index;
	BYTE MaterialIndex;
}MS3D_MESH;

typedef struct _MS3D_MATERIAL
{
	char Name[32];
	float Ambient[4];
	float Diffuse[4];
	float Specular[4];
	float Emissive[4];
	float Shinness;
	float Transparency;
	char  Mode;
	char  TextrueFile[128];
	char  AlphaFile[128];
}MS3D_MATERIAL;

typedef struct _MS3D_KEY_FRAME
{
	float time;
	float Param[3];
}MS3D_KEY_FRAME;

typedef struct _MS3D_JOINT
{
	// 93 BYTE
	unsigned char flag;
	char Name[32];
	char Parent[32];
	float init_Rotation[3];
	float init_Translation[3];
	WORD  num_Rotation;
	WORD  num_Translation;

	// ext memeber
	MS3D_KEY_FRAME * f_Rot;
	MS3D_KEY_FRAME * f_Trans;
	WORD Parent_ID;
	D3DXMATRIX Matr_Local;
	D3DXMATRIX Matr_Abs;
	D3DXMATRIX Matr_Fanal;
	D3DXMATRIX Matr_InitLocal;
	WORD  CurrentTransIndex;
	WORD  CurrentRotIndex;
}MS3D_JOINT;

}}//end namespace


#pragma pack()
#endif