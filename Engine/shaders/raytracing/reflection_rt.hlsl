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
void ClosestHit(inout ShadingRayPayload payload, in SimpleAttributes attr)
{
    HitPointContext hitPoint = GetHitPointContext(attr);
    HitPointMaterial material = GetHitPointMaterial(hitPoint);

    payload.Position = hitPoint.WorldSpacePosition;
    payload.Normal = hitPoint.WorldSpaceNormal;
    payload.LookVector = hitPoint.WorldSpaceLookVector;
    payload.Diffuse = material.Diffuse;
    payload.Specular = material.Specular;
    payload.Roughness = material.Roughness;
    payload.Metallic = material.Metallic;
    payload.Hit = true;
}

[shader("miss")]
void Miss(inout ShadingRayPayload payload)
{
    payload.Hit = false;
    payload.Diffuse = gLightProbe.SampleLevel(gSam, WorldRayDirection(), 0);
}

#endif // RAYTRACING_HLSL
