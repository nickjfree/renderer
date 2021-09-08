#ifndef __LIGHTING_RT__
#define __LIGHTING_RT__

#include "raytracing.hlsli"
#include "shading.hlsli"

// output
RWTexture2D<float4> RenderTarget : register(u0, space0);



[shader("raygeneration")]
void Raygen()
{
    float2 uv = (float2)(DispatchRaysIndex().xy + 0.5)/DispatchRaysDimensions().xy;
    uint linearIndex = DispatchRaysIndex().x + DispatchRaysIndex().y * DispatchRaysDimensions().x;   

    GBufferContext gbuffer = GetGBufferContext(uv);
    // ignore background pixels
    if (length(gbuffer.ViewSpacePosition) < 0.001) {
        RenderTarget[DispatchRaysIndex().xy] = float4(0, 0, 0, 1);
        return;
    }
    // direct lighting
    RayContext ray;
    ray.Seed = RandInit(linearIndex, gFrameNumber);
    // direct lighting
    float4 direct = ComputeDirectLighting(gbuffer, ray);
    // indirect lighting
    float4 indirect = ComputeIndirectLighting(gbuffer);
    // indirect = 0;
    RenderTarget[DispatchRaysIndex().xy] = direct + indirect;
}

[shader("closesthit")]
void ClosestHit(inout ShadowRayPayload payload, in SimpleAttributes attr)
{
    payload.Color = float4(0, 0, 0, 0);
}

[shader("miss")]
void Miss(inout ShadowRayPayload payload)
{
    // payload.color = float4(0, 0, 0, 1);
    payload.Color = float4(1, 1, 1, 1);
}

#endif // RAYTRACING_HLSL