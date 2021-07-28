#ifndef __REFLECTION_RT__
#define __REFLECTION_RT__

#include "raytracing.hlsli"
#include "random.hlsli"
#include "sampling.hlsli"


#include "../common/gbuffer.hlsli"


// screen color
Texture2D   gPostBuffer : register(t1, space0);
// env map
TextureCube gLightProbe : register(t2, space0);
// output
RWTexture2D<float4> RenderTarget : register(u0, space0);

struct RayPayload
{
    // color or screen uv
    float4 Color;
};


void TraceReflectionRay(GBufferContext gbuffer, inout RayContext rayContext, inout RayPayload payload)
{
    float3 origin = gbuffer.WorldSpacePosition;
    float3 look = -gbuffer.WorldSpaceLookVector;
    float3 normal = gbuffer.WorldSpaceNormal;
    float roughness = gbuffer.Roughness;

    float2 randsample = float2(Rand(rayContext.Seed), Rand(rayContext.Seed));
    float4 sample = GenerateReflectedRayDirection(look, normal, roughness, randsample);
    float3 rayDir = sample.xyz;
    float invPDF = sample.w;
    FixSampleDirectionIfNeeded(normal, rayDir);
    // get ray
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = normalize(rayDir);
    // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
    // TMin should be kept small to prevent missing geometry at close contact areas.
    ray.TMin = 0.005;
    ray.TMax = 10000.0;
    TraceRay(Scene, RAY_FLAG_FORCE_OPAQUE, ~0, 0, 1, 0, ray, payload);
}

[shader("raygeneration")]
void Raygen()
{
    float2 uv = (float2)(DispatchRaysIndex().xy + 0.5)/DispatchRaysDimensions().xy;
    uint linearIndex = DispatchRaysIndex().x + DispatchRaysIndex().y * DispatchRaysDimensions().x;   

    GBufferContext gbuffer = GetGBufferContext(uv);

    // ignore background
    if (length(gbuffer.ViewSpacePosition) < 0.001) {
        RenderTarget[DispatchRaysIndex().xy] = float4(0, 0, 0, 1);
        return;
    }

    RayContext ray;
    ray.Seed = RandInit(linearIndex, gFrameNumber);

    RayPayload payload;
    payload.Color = float4(0, 0, 0, 0);
    // trace reflection ray
    TraceReflectionRay(gbuffer, ray, payload);
    // output
    RenderTarget[DispatchRaysIndex().xy] = payload.Color;
}

[shader("closesthit")]
void ClosestHit(inout RayPayload payload, in SimpleAttributes attr)
{
    HitPointContext hitPoint = GetHitPointContext(attr);
    if (hitPoint.InScreen) {
        // get gbuffer at hitpoint in screen
        GBufferContext gbuffer = GetGBufferContext(hitPoint.ScreenUV);
        if (hitPoint.ViewSpacePosition.z - gbuffer.ViewSpacePosition.z < 0.1) {
            // close to point in screen. get color from screen
            payload.Color = gPostBuffer.SampleLevel(gSam, hitPoint.ScreenUV, 0);
        }
    } else {
        HitPointMaterial material = GetHitPointMaterial(hitPoint);
        payload.Color = float4(material.Diffuse.xyz, 0);
    }
}

[shader("miss")]
void Miss(inout RayPayload payload)
{
    payload.Color = gLightProbe.SampleLevel(gSam, WorldRayDirection(), 0);
}

#endif // RAYTRACING_HLSL
