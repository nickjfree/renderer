#ifndef __SHADER_DATA__
#define __SHADER_DATA__

typedef struct float2 
{
	float x, y;
} float2;

typedef struct float3 
{
	float x, y, z;
} float3;

typedef Vector3 float4;

// matrix4x4
typedef Matrix4x4 float4x4;

// per object constant buffer
typedef struct PerObject 
{

	// return slot
	static int Slot() { return 0; }
	// return space
	static int Space() { return 0; }
	// return name
	const char* Name() { return "PerObject"; }

	// object
	float4x4 gWorldViewProjection;
	float4x4 gWorldViewMatrix;
	// prev
	float4x4 gPrevWorldViewProjection;
	// material
	float gSpecular;
	// object id
	int   gObjectId;
	// pad
	float2 pad_1;
}PerObject;


typedef  struct SkinningMatrices
{
	// return slot
	static int Slot() { return 4; }
	// return space
	static int Space() { return 0; }
	// return name
	const char* Name() { return "ArrayKeyframe"; }

	float4x4  Matrices[128];
} SkinningMatrices;


#endif