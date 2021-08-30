#ifndef __REFLECTION_RT__
#define __REFLECTION_RT__

#include "raytracing.hlsli"
#include "shading.hlsli"
#include "../common/gi/probe.hlsli"
#include "../common/gi/operations.hlsli"



// env map
TextureCube gLightProbe : register(t3, space0);

// output
RWTexture2D<float4> RenderTarget : register(u0, space0);

[shader("raygeneration")]
void Raygen()
{
    int probeIndex = DispatchRaysIndex().y;
    int rayIndex = DispatchRaysIndex().x;

    uint linearIndex = DispatchRaysIndex().x + DispatchRaysIndex().y * DispatchRaysDimensions().x;   
    uint seed = RandInit(linearIndex, gFrameNumber);
    // get ray diraction and position
    float3 direction = SphericalFibonacci(rayIndex, CBGIVolume.numRaysPerProbe);
    float3 position = GetProbePosition(probeIndex);
    // seed
    RayContext ray;
    ray.Seed = RandInit(linearIndex, gFrameNumber);
    float4 result = ComputeGIProbeTracingRadiance(position, normalize(direction), ray);
    RenderTarget[DispatchRaysIndex().xy] = result;
}


[shader("closesthit")]
void ClosestHit(inout ShadingRayPayload payload, in SimpleAttributes attr)
{
    payload.Hit = true;
}


[shader("miss")]
void Miss(inout ShadingRayPayload payload)
{
    payload.Hit = false;
    payload.Diffuse = gLightProbe.SampleLevel(gSam, WorldRayDirection(), 0);
    payload.HitDistance = 999999999;
}

#endif // RAYTRACING_HLSL
