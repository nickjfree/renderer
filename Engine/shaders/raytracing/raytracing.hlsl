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
#include "../common/post.h"
#include "random.hlsli"
#include "monte_carlo.hlsli"

RaytracingAccelerationStructure Scene : register(t0, space0);
RWTexture2D<float4> RenderTarget : register(u0, space0);
// Texture2D PrevRenderTarget : register(t1, space0);


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

    float3 specular = gbuffer.Specular;

    float3 world_normal = mul(float4(normal, 0), gInvertViewMaxtrix).xyz;
    float3 world_look =  mul(float4(look, 0), gInvertViewMaxtrix).xyz;
    // random seed
    uint seed = RandInit(linearIndex, gFrameNumber);
    float2 randsample = float2(Rand(seed), Rand(seed));
    float4 sample = GenerateReflectedRayDirection(world_look, world_normal, roughness, randsample);
    float3 rayDir = sample.xyz;
    float invPDF = sample.w;
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
    // get raytraced color
    TraceRay(Scene, RAY_FLAG_FORCE_OPAQUE, ~0, 0, 1, 0, ray, payload);
    // target
    RenderTarget[DispatchRaysIndex().xy] = float4(payload.color.xyz, 0);
    // RenderTarget[DispatchRaysIndex().xy] = float4(0, 0, 0, 0);
}


[shader("closesthit")]
void ClosestHit(inout RayPayload payload, in SimpleAttributes attr)
{
    if (InstanceID() == 1) {
        payload.color = float4(0, 50, 0, RayTCurrent());
        return;
    }
    // try get the color from screen space
    float3 hitPosition = WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
    float4 screenPosition = mul(float4(hitPosition, 1), gViewProjectionMatrix);
    screenPosition.x = screenPosition.x / screenPosition.w * 0.5 + 0.5;
    screenPosition.y = -screenPosition.y / screenPosition.w * 0.5 + 0.5;
    // get gbuffer 
    if ((saturate(screenPosition.x) == screenPosition.x) && (saturate(screenPosition.y) == screenPosition.y)) {
        // in screen space
        GBuffer gbuffer = GetGBufferLoad(screenPosition.xy);
        float hitDepth = mul(float4(hitPosition, 1), gViewMatrix).z;
        float gbufferDepth = gbuffer.Position.z;
        if (hitDepth - gbufferDepth < 0.1) {
            payload.color = gPostBuffer.SampleLevel(gSam, screenPosition.xy, 0);
            return;
        }
    }
    // can't get color from screen space, get it from textures
    float4 color = SampleHitPointColor(attr);

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