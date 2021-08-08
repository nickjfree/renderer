#ifndef __RASTERIZATION_COMMON__
#define __RASTERIZATION_COMMON__


#include "../common/shader_inputs.h"

#ifdef RENDER_GBUFFER
	#include "../common/gbuffer.hlsli"
#endif

#if defined(VS_BLENDSHAPE) || defined(VS_SKINNING)
	#include "animation.hlsli"
#endif

#ifdef VS_CLIPMAP
	// terrain clipmap rendering
	#include "terrain.hlsli"
#endif

/*
*  vertex input layout 
*/

#ifdef RENDER_SCREEN
// screen space rendering
struct VS_Input
{
    float3 PosL  : POSITION;
    float2 TexCoord : TEXCOORD;
};

#elif VS_SKINNING

// skinning
struct VS_Input
{
    float3 PosL  : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    uint  Bones : TEXCOORD1;
    float3 Weight : TEXCOORD2;
    float3 Tangent  : TANGENT;
};

#elif VS_INSTANCING

// instancing
struct VS_Input
{
    float3 PosL  : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    float  InstanceID : TEXCOORD1;
    float3 Tangent  : TANGENT;
    float4x4 InstanceWVP: InstanceWVP; 
    float4x4 InstanceWV: InstanceWV;
    float4x4 InstancePWVP: InstancePWVP;
    int InstanceObjectId: InstanceObjectId;
};

#else

// default
struct VS_Input
{
    float3 PosL  : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    float  InstanceID : TEXCOORD1;
    float3 Tangent  : TANGENT;
};

#endif



/* 
*   pixel shader input
*/

#ifdef RENDER_SCREEN

// screen space
struct PS_Input
{
    float4 PosH : SV_POSITION;
    float4 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

#elif RENDER_GBUFFER

// gbuffer
struct PS_Input
{
	float4 PosH : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
	float4 Normal : NORMAL;
	float4 Tangent : TEXCOORD1;
	float4 BiNormal: TEXCOORD2;
	float4 PrevPosH:  TEXCOORD3;
	float4 CurrentPosH:  TEXCOORD4;
	float  Depth : TEXCOORD5;
	int    ObjectId : TEXCOORD6;
};

#else

// default
struct PS_Input
{
    float4 PosH : SV_POSITION;
    float4 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

#endif


/* 
*   pixel shader output
*/
#ifdef RENDER_GBUFFER

struct PS_Output
{

	float4 Diffuse : SV_Target0;
	float4 Compact: SV_Target1;   //  xy: normal  z: linearZ(half)  w: objectId,
	float4 Depth: SV_Target2;     // linearZ
	float4 Specular: SV_Target3;
	float4 Motion: SV_Target4;
};

#elif SVGF_ACC

struct PS_Output
{
	float4 Color   : SV_Target0;
    float4 Moment  : SV_Target1;
};

#else

struct PS_Output
{
	float4 Color : SV_Target0;
};

#endif


// return position in viewspace
PS_Input transform_to_view(
	float3 position, float3 normal, float2 texcoord, float3 tangent,
	float4x4 mvp, float4x4 mv, float4x4 pmvp, int objectId)
{
	// the output
	PS_Input output = (PS_Input)0;
	// get view space position
	float4 viewPosition = mul(float4(position.xyz, 1.0f), mv);
	// transform to homogeneous clip space.
	output.PosH = mul(float4(position, 1.0f), mvp);
	output.TexCoord = texcoord;
	output.Normal = float4(normal, 0);

#ifdef RENDER_GBUFFER
	output.CurrentPosH = output.PosH;
	output.PrevPosH = mul(float4(position, 1.0f), pmvp);
	// processing normal tangent and bi-normal
	output.Normal = mul(float4(normal, 0), mv);
	output.Tangent = mul(float4(tangent, 0), mv);
	output.BiNormal = float4(cross(output.Normal.xyz, output.Tangent.xyz), 0);
	output.BiNormal = normalize(output.BiNormal);
	output.Normal = normalize(output.Normal);
	output.Tangent = normalize(output.Tangent);
	// depth is z value in view space
	output.Depth = viewPosition.z;
	// object id
	output.ObjectId = objectId;
#endif

	return output;
}


#endif