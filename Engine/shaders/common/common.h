#ifndef __COMMON__
#define __COMMON__

#include "basic_sampler.hlsli"
#include "basic_constant.h"
#include "basic_registers.h"
#include "basic_layout.h"

/*
    transform vertex from local space to view space for gbuffer
*/

PS_Input_GBuffer transform_to_view_gbuffer(
    float3 position, float3 normal, float2 texcoord, float3 tangent,
    float4x4 mvp, float4x4 mv, float4x4 pmvp)
{
    // the output
    PS_Input_GBuffer output = (PS_Input_GBuffer)0;
    // get view space position
    float4 view_position = mul(float4(position.xyz, 1.0f), mv);
    // transform to homogeneous clip space.
    output.PosH = mul(float4(position, 1.0f), mvp);

    output.CurrentPosH = output.PosH;
    output.PrevPosH = mul(float4(position, 1.0f), pmvp);
    // processing normal tangent and bi-normal
    output.Normal = mul(float4(normal, 0), mv);
    output.Tangent = mul(float4(tangent, 0), mv);
    output.BiNormal = float4(cross(output.Normal.xyz, output.Tangent.xyz), 0);
    output.BiNormal = normalize(output.BiNormal);
    output.Normal = normalize(output.Normal);
    output.Tangent = normalize(output.Tangent);
    output.TexCoord = texcoord;
    // depth is z value in view space
    output.Depth = view_position.z;
    return output;
}

/*
    transform vertex from local space to view space for simple rendering
*/

PS_Input_Simple transform_to_view_simple(
    float3 position, float3 normal, float2 texcoord, float3 tangent,
    float4x4 mvp, float4x4 mv)
{
    // the output
    PS_Input_Simple output = (PS_Input_Simple)0;
    // transform to homogeneous clip space.
    output.PosH = mul(float4(position, 1.0f), mvp);
    output.TexCoord = texcoord;
    output.Normal = normal;
    return output;
}


#endif