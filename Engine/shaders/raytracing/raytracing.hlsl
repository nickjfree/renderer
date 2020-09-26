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
#include "random.hlsli"
#include "monte_carlo.hlsli"

RaytracingAccelerationStructure Scene : register(t0, space0);
RWTexture2D<float4> RenderTarget : register(u0, space0);
// Texture2D PrevRenderTarget : register(t1, space0);

ByteAddressBuffer Vertices : register(t0, space1);
ByteAddressBuffer Indices : register(t1, space1);
Texture2D gDiffuseMap0 : register(t2, space1);
Texture2D gNormalMap0 : register(t3, space1);
Texture2D gSpecularMap0 : register(t4, space1);

// light prob
TextureCube  gLightProbe           : register(t17);

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
    uint linearIndex = DispatchRaysIndex().x + DispatchRaysIndex().y * DispatchRaysDimensions().x;   

    GBuffer gbuffer = GetGBufferLoad(uv);

    float3 viewPos = gbuffer.Position.xyz;
    // world position to sun 
    float3 origin = mul(float4(viewPos, 1), gInvertViewMaxtrix).xyz;
    // get view-space normal
    float3 normal = gbuffer.Normal.xyz; 
    // get view-space look
    float3 look = -gbuffer.View.xyz;

    float roughness = gbuffer.Roughness;

    float3 world_normal = mul(float4(normal, 0), gInvertViewMaxtrix).xyz;
    float3 world_look =  mul(float4(look, 0), gInvertViewMaxtrix).xyz;
    // random seed
    uint seed = RandInit(linearIndex, gFrameNumber);
    float2 randsample = float2(Rand(seed), Rand(seed));
    float3 rayDir = GenerateReflectedRayDirection(world_look, world_normal, roughness, randsample);
    FixSampleDirectionIfNeeded(world_normal, rayDir);
    // Trace the ray.
    // Set the ray's extents.
    // float4 look = GetLookVector(uv);
    // float3 rayDir = mul(look, gInvertViewMaxtrix).xyz;
    // float3 origin = mul(float4(0, 0, 0, 1), gInvertViewMaxtrix).xyz;

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

    // // Write the raytraced color to the output texture.

    RenderTarget[DispatchRaysIndex().xy] = payload.color;

    // RenderTarget[DispatchRaysIndex().xy] = float4(1,1,1,1);
    // float history;
    // float2 prevUV = GetPrevScreenCoordLoad(uv, history);
    // float4 prevColor = PrevRenderTarget.SampleLevel(gSamPoint, prevUV, 0);
    // float alpha = max(0.05, 1.0 / history);
    // RenderTarget[DispatchRaysIndex().xy] = lerp(prevColor, payload.color, alpha);
}


[shader("closesthit")]
void ClosestHit(inout RayPayload payload, in MyAttributes attr)
{
    if (InstanceID() == 1) {
        payload.color = float4(0, 100, 0, RayTCurrent());
        return;
    }
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
    payload.color = float4(color.xyz, RayTCurrent());
}

[shader("miss")]
void Miss(inout RayPayload payload)
{
    // payload.color = float4(0, 0, 0, 1);
    payload.color = gLightProbe.SampleLevel(gSam, WorldRayDirection(), 0);
}

#endif // RAYTRACING_HLSL