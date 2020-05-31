//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL

// #include "RaytracingHlslCompat.h"
#include "../common/raytracing.hlsli"

RaytracingAccelerationStructure Scene : register(t0, space0);
RWTexture2D<float4> RenderTarget : register(u0, space0);
ByteAddressBuffer Vertices : register(t0, space1);
ByteAddressBuffer Indices : register(t1, space1);

Texture2D gDiffuseMap0 : register(t2, space1);
Texture2D gNormalMap0 : register(t3, space1);
Texture2D gSpecularMap0 : register(t4, space1);
Texture2D gNormalBuffer1 : register(t5, space1);



// info about the instance
cbuffer InstanceInfo: register(b0, space1)
{
    // size of the vertex
    uint gVertexStride;
}

typedef BuiltInTriangleIntersectionAttributes MyAttributes;



struct RayPayload
{
    float4 color;
};

[shader("raygeneration")]
void Raygen()
{

    float2 uv = (float2)(DispatchRaysIndex().xy + 0.5)/DispatchRaysDimensions().xy;
    float3 viewPos = GetPositionLoad(uv).xyz;
    // word position to sun 
    float3 origin = mul(float4(viewPos, 1), gInvertViewMaxtrix).xyz;
    // float3 origin = mul(float4(GetPositionLoad(DispatchRaysIndex().xy), 1), gInvertViewMaxtrix).xyz;
    // get view normal
    float3 normal = GetNormalLoad(uv).xyz; 
    // get view space look
    float3 look = GetLookVector(uv).xyz;
    // get view space raydir
    float3 rayDir = reflect(look, normal);
    // get world space ray
    rayDir = mul(float4(rayDir, 0), gInvertViewMaxtrix).xyz;
    // normalize(float3(1, 1, 0));
    // Trace the ray.
    // Set the ray's extents.
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = normalize(rayDir);
    // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
    // TMin should be kept small to prevent missing geometry at close contact areas.
    ray.TMin = 0.005;
    ray.TMax = 10000.0;
    RayPayload payload = { float4(0, 0, 0, 0) };

    if (length(viewPos) < 0.001) {
        RenderTarget[DispatchRaysIndex().xy] = float4(0, 0.0, 0, 1);
        return;
    }

    TraceRay(Scene, RAY_FLAG_NONE, ~0, 0, 1, 0, ray, payload);

    // Write the raytraced color to the output texture.
    RenderTarget[DispatchRaysIndex().xy] = payload.color;

    // RenderTarget[DispatchRaysIndex().xy] = float4(origin, 1);
}

[shader("closesthit")]
void ClosestHit(inout RayPayload payload, in MyAttributes attr)
{
    
    // 
    float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);
    // PrimitiveIndex
    uint index0 = PrimitiveIndex() * 3;

    // get the triangle indices
    uint3 indices = LoadIndices16Bit(Indices, index0 * 2);
    // get the triangle uvs

    float2 uv0 = LoadVertexUVFloat2(Vertices, indices[0], gVertexStride);
    float2 uv1 = LoadVertexUVFloat2(Vertices, indices[1], gVertexStride);
    float2 uv2 = LoadVertexUVFloat2(Vertices, indices[2], gVertexStride);

    float2 uv = uv0 * barycentrics.x + uv1 * barycentrics.y + uv2 * barycentrics.z;

    float4 color = gDiffuseMap0.SampleLevel(gSam, uv, 0);

   // float4 color = float4(uv, 0.0, 0);

    payload.color = color;
    // if (gVertexStride == 48) {

    //     StructuredBuffer<VS_Input_Simple> vertex = Vertics;

    //     payload.color = float4(0, 1.0, 0.0, 1);
    // } else {

    //     StructuredBuffer<VS_Input_Skinning> vertex = (StructuredBuffer<VS_Input_Skinning>)Vertics;

    //     payload.color = float4(1.0, 1.0, 1.0, 1);
    // }



    
}

[shader("miss")]
void Miss(inout RayPayload payload)
{
    payload.color = float4(0, 0.0, 0, 1);
}

#endif // RAYTRACING_HLSL