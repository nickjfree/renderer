#ifndef __SHADER_TYPES__
#define __SHADER_TYPES__


#ifdef __cplusplus // c++ side

#include "Math/LinearMath.h"


typedef Matrix4x4 float4x4;

typedef struct float2 {
	float x, y;
}float2;

typedef struct float3 {
	float x, y, z;
}float3;

typedef Vector3 float4;

typedef unsigned int uint;

typedef struct uint2 {
	unsigned int x, y;
}uint2;

typedef struct uint3 {
	unsigned int x, y, z;
}uint3;

typedef struct uint4 {
	unsigned int x, y, z, w;
}uint4;



#define CB_SLOT(name) __CBSLOT__##name##__
#define CBUFFER(name, slot) constexpr int CB_SLOT(name) = slot; struct alignas(16) name
#define SAMPLERSTATE(name, slot)

#else   // hlsl side

#define CBUFFER(name, slot) cbuffer name : register(b ## slot)
#define SAMPLERSTATE(name, slot) SamplerState name : register(s ## slot)


#endif // __cplusplus



#endif // __SHADER_TYPES__