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
#define CELL_SCALE  1
#define CELL_COUNT  16

cbuffer ArraylightInfos: register(b0, space0)
{
    LightData lights[256];
//    float4x4 padxxx;
}


StructuredBuffer<LightIndics> CulledLights : register(t1, space0);

struct RayPayload
{
    float4 color;

};


uint GetLightBufferIndex(float3 position, float3 viewPoint) 
{
    position -= viewPoint;
    position /= CELL_SCALE;
    // sign
    float3 region = sign(position);
    position = abs(position);
    position = max(position, 2);
    position = min(log2(position) - 1, CELL_COUNT/2 - 1);
    // get the center
    position = floor(position) + 0.5;
    position = position * region;
    // id
    float3 center = floor(position + CELL_COUNT/2);
    // get index
    uint address = (center.x + CELL_COUNT * (center.y + (center.z * CELL_COUNT)));

    return  address;
}



void TraceShadowRay(float3 origin, float3 look, float3 normal, float3 target, float roughness, uint seed, inout RayPayload payload) 
{
    // float2 randsample = float2(Rand(seed), Rand(seed));
    float4 sample = normalize(float4(1, 1, 1, 0));
    float3 rayDir = target;
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

    // get world position
    float3 origin = mul(float4(viewPos, 1), gInvertViewMaxtrix).xyz;
    // get lights affecting this area
    uint addr = GetLightBufferIndex(origin, gViewPoint.xyz);
    if (CulledLights[addr].numLights == 0) {
        // no lights. return black
        RenderTarget[DispatchRaysIndex().xy] = float4(0, 0, 0, 1);
        return;
    }

    // deferred lighting
    float falloff = 1.0f;
    float3 lighting_color = get_lighting(lights[0], gbuffer, falloff);
    if (falloff <= 0.001f) {
         RenderTarget[DispatchRaysIndex().xy] = float4(0, 0, 0, 1);
         return;
    }
    
    // get view-space normal
    float3 normal = gbuffer.Normal.xyz; 
    // get view-space look
    float3 look = -gbuffer.View.xyz;
    // transform to world space
    float3 world_normal = mul(float4(normal, 0), gInvertViewMaxtrix).xyz;
    float3 world_look =  mul(float4(look, 0), gInvertViewMaxtrix).xyz;
    // random seed
    uint seed = RandInit(linearIndex, gFrameNumber);

    float4 color = float4(0,0,0,0);
    RayPayload payload;
    payload.color = color;
    // tracy shadow rays
    // TODO: ignore closest hit shaders and do lighting in raygen shader
    float roughness = gbuffer.Roughness;
    //float3 target = (lights[0].xyz - origin) * numLights;
    float3 target = lights[0].position.xyz - origin;
    TraceShadowRay(origin, world_look, world_normal, target, roughness, seed, payload);
    // test sun light
    float3 L = float3(1, 1, 1);
    L = mul(float4(normalize(L),0), gViewMatrix).xyz;

    RenderTarget[DispatchRaysIndex().xy] = payload.color * float4(lighting_color, 0);

// #ifdef DEBUG_CULLED_LIGHT

//     float mark = CulledLights[addr].numLights;
//     float x = addr % 4;
//     float y = (addr/16/16) % 4;
//     float4 debug = float4(0, 0, 0, 0);
//     if (x == 0) {
//         debug += float4(1, 0, 0, 0);
//     }
//     if (x == 1) {
//         debug += float4(0, 1, 0, 0);
//     }
//     if (x == 2) {
//         debug += float4(0, 0, 1, 0);
//     }
//     if (x == 3) {
//         debug += float4(1, 1, 0, 0);
//     }
//     if (y == 0) {
//         debug += float4(0, 0, 1, 0);
//     }
//     if (y == 1) {
//         debug += float4(0, 1, 0, 0);
//     }
//     if (y == 2) {
//         debug += float4(1, 0, 0, 0);
//     }
//     if (y == 3) {
//         debug += float4(0, 1, 1, 0);
//     }
//     RenderTarget[DispatchRaysIndex().xy] = debug * mark * 0.5;
// #endif

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