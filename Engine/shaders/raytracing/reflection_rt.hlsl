#ifndef __REFLECTION_RT__
#define __REFLECTION_RT__

#include "raytracing.hlsli"
#include "shading.hlsli"


// screen color
Texture2D   gPostBuffer : register(t2, space0);
// env map
TextureCube gLightProbe : register(t3, space0);
// output
RWTexture2D<float4> RenderTarget : register(u0, space0);

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

    float4 reflection = ComputeReflectionLighting(gbuffer, ray);
    // float4 reflection = float4(0,0,0,0);
    // output
    RenderTarget[DispatchRaysIndex().xy] = reflection;
}


[shader("closesthit")]
void ClosestHit(inout ReflectionRayPayload payload, in SimpleAttributes attr)
{
    HitPointContext hitPoint = GetHitPointContext(attr);
    // if (hitPoint.InScreen) {
    //     // get gbuffer at hitpoint in screen
    //     GBufferContext gbuffer = GetGBufferContext(hitPoint.ScreenUV);
    //     if (hitPoint.ViewSpacePosition.z - gbuffer.ViewSpacePosition.z < 0.1) {
    //         // close to point in screen. get color from screen
    //         payload.Color = gPostBuffer.SampleLevel(gSam, hitPoint.ScreenUV, 0);
    //         return;
    //     }
    // }
    // failed to get color from screen space
    // HitPointMaterial material = GetHitPointMaterial(hitPoint);
    // payload.Color = float4(material.Diffuse.xyz, 0);
    // compute hitPoint lighting
    float2 uv = (float2)(DispatchRaysIndex().xy + 0.5)/DispatchRaysDimensions().xy;
    uint linearIndex = DispatchRaysIndex().x + DispatchRaysIndex().y * DispatchRaysDimensions().x; 

    RayContext ray;
    ray.Seed = RandInit(linearIndex, gFrameNumber);

    GBufferContext gbuffer = HitPointToGBufferContext(hitPoint);
    payload.Color = ComputeDirectLighting(gbuffer, ray);
}

[shader("miss")]
void Miss(inout ReflectionRayPayload payload)
{
    payload.Color = gLightProbe.SampleLevel(gSam, WorldRayDirection(), 0);
}

#endif // RAYTRACING_HLSL
