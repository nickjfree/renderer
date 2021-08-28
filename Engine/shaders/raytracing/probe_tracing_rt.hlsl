#ifndef __REFLECTION_RT__
#define __REFLECTION_RT__

#include "raytracing.hlsli"
#include "shading.hlsli"
#include "gi/probe.hlsli"


// env map
TextureCube gLightProbe : register(t3, space0);

// output
RWTexture2D<float4> RenderTarget : register(u0, space0);

[shader("raygeneration")]
void Raygen()
{
    int probeIndex = DispatchRaysDimensions().y;
    int rayIndex = DispatchRaysDimensions().x;

    uint linearIndex = DispatchRaysIndex().x + DispatchRaysIndex().y * DispatchRaysDimensions().x;   
    RayContext ray;
    ray.Seed = RandInit(linearIndex, gFrameNumber);
    // get ray diraction and position
    float3 direction;
    float3 position;
    // traceRay
    // float4 reflection = float4(0,0,0,0);
    // output
    // RenderTarget[DispatchRaysIndex().xy] = reflection;
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
