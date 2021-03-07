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
#include "raytracing.hlsli"
#include "../common/post.h"
#include "random.hlsli"
#include "monte_carlo.hlsli"
#include "../lighting/lighting.hlsli"

RaytracingAccelerationStructure Scene : register(t0, space0);
RWTexture2D<float4> RenderTarget : register(u0, space0);
// Texture2D PrevRenderTarget : register(t1, space0);

struct LightIndics
{
    uint numLights;
    uint lightIndics[15];
};

#define MAX_LIGHT_COUNT_PER_CELL  16

StructuredBuffer<LightIndics> CulledLights : register(t1, space0);




struct RayPayload
{
    float4 color;

};


void TraceShadowRay(float3 origin, float3 look, float3 normal, float roughness, uint seed, inout RayPayload payload) 
{
    // float2 randsample = float2(Rand(seed), Rand(seed));
    float4 sample = normalize(float4(1, 1, 1, 0));
    float3 rayDir = sample.xyz;
    // get ray
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = normalize(rayDir);
    // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
    // TMin should be kept small to prevent missing geometry at close contact areas.
    ray.TMin = 0.005;
    ray.TMax = 10000.0;
    TraceRay(Scene, RAY_FLAG_FORCE_OPAQUE, ~0, 1, 1, 1, ray, payload);
}



[shader("raygeneration")]
void Raygen()
{
    float2 uv = (float2)(DispatchRaysIndex().xy + 0.5)/DispatchRaysDimensions().xy;
    uint linearIndex = DispatchRaysIndex().x + DispatchRaysIndex().y * DispatchRaysDimensions().x;   

    GBuffer gbuffer = GetGBufferLoad(uv);

    float3 viewPos = gbuffer.Position.xyz;

    if (length(viewPos) < 0.001) {
        RenderTarget[DispatchRaysIndex().xy] = float4(0, 0, 0, 1);
        return;
    }
  
    // get view-space normal
    float3 normal = gbuffer.Normal.xyz; 
    // get view-space look
    float3 look = -gbuffer.View.xyz;

    float roughness = gbuffer.Roughness;
  // world position to sun 
    float3 origin = mul(float4(viewPos, 1), gInvertViewMaxtrix).xyz;
    float3 world_normal = mul(float4(normal, 0), gInvertViewMaxtrix).xyz;
    float3 world_look =  mul(float4(look, 0), gInvertViewMaxtrix).xyz;
    // random seed
    uint seed = RandInit(linearIndex, gFrameNumber);

    float4 color = float4(0,0,0,0);
    RayPayload payload;
    payload.color = color;

    TraceShadowRay(origin, world_look, world_normal, roughness, seed, payload);
    // sun light
    float3 L = float3(1, 1, 1);
    L = mul(float4(normalize(L),0), gViewMatrix).xyz;
    // deferred lighting
    float3 lighting_color = 3 * float3(1, 1, 1) * deferred_lighting(gbuffer, L).xyz;
    // test shadow
    uint addr = linearIndex % 4096;
    RenderTarget[DispatchRaysIndex().xy] = payload.color * float4(lighting_color, CulledLights[addr].numLights);
    // RenderTarget[DispatchRaysIndex().xy] = float4(0, 0, 0, 0);
}


[shader("closesthit")]
void ClosestHit(inout RayPayload payload, in SimpleAttributes attr)
{
    payload.color = float4(0, 0, 0, 1);
}

[shader("miss")]
void Miss(inout RayPayload payload)
{
    // payload.color = float4(0, 0, 0, 1);
    payload.color = float4(1, 1, 1, 1);
}

#endif // RAYTRACING_HLSL