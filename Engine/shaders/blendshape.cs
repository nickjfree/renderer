#ifndef __VSNORMAL__
#define __VSNORMAL__

#include "blendshape.h"


struct VS_Input
{
    float3 PosL  : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    float  InstanceID : TEXCOORD1;
    float4 Tangent  : TANGENT;
};

struct PS_Input
{
    float4 PosH : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Normal : NORMAL;
    float4 Tangent : TEXCOORD1;
    float4 BiNormal: TEXCOORD2;
    float  Depth :TEXCOORD3;
};

struct PS_Input_Basic
{
    float4 PosH : SV_POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord :TEXCOORD0;
};

struct PS_Output
{
    float4 Depth : SV_Target0;
    float4 Normal: SV_Target1;
    float4 Diffuse: SV_Target2;
    float4 Specular: SV_Target3;
};

PS_Input VS_LPP(VS_Input input)
{
    PS_Input output = (PS_Input)0;
    
    // blending multiple shapes
    input.PosL = input.PosL;
    // Transform to homogeneous clip space.
    output.PosH = mul(float4(input.PosL, 1.0f), gWorldViewProjection);
    float4 pos = mul(float4(input.PosL,1.0f), gWorldViewMatrix);
    output.Normal = mul(float4(input.Normal, 0), gWorldViewMatrix);
    output.Tangent = mul(float4(input.Tangent.xyz, 0), gWorldViewMatrix);
    output.BiNormal = float4(cross(output.Normal.xyz, output.Tangent.xyz),0);
    output.BiNormal = normalize(output.BiNormal);
    output.Normal = normalize(output.Normal);
    output.Tangent = normalize(output.Tangent);
    output.TexCoord = input.TexCoord;
    output.Depth = pos.z;
    return output;
}



PS_Input_Basic VS(VS_Input input)
{
    PS_Input_Basic output = (PS_Input_Basic)0;
    // blending multiple shapes

    output.PosH = mul(float4(input.PosL, 1.0f), gWorldViewProjection);
    float4 pos = mul(float4(input.PosL,1.0f), gWorldViewMatrix);
    output.Normal = mul(float4(input.Normal, 0), gWorldViewMatrix);
    output.Normal = normalize(output.Normal);
    output.TexCoord = input.TexCoord;
    return output;
}


#endif