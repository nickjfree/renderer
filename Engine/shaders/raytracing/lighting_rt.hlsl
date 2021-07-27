#ifndef __LIGHTING_RT__
#define __LIGHTING_RT__

#include "raytracing.hlsli"
#include "random.hlsli"
#include "sampling.hlsli"


#include "../common/gbuffer.hlsli"
#include "../common/lighting.hlsli"

// output
RWTexture2D<float4> RenderTarget : register(u0, space0);
// culled lights
StructuredBuffer<LightIndics> CulledLights : register(t1, space0);


struct RayPayload
{
    // color or screen uv
    float4 Color;
};


// get random shadow ray direcion
float3 GetShadowRaySample(LightData light, float3 position, inout uint seed)
{
    float3 ray = float3(0, 0, 0);
    if (light.Type == LIGHT_TYPE_SPHERE) {
        // point light 
        // test code set a radius to test a sphere light
        float3 p = UniformSampleSphere(Rand(seed), Rand(seed));
        ray = light.Position.xyz + p * 1 - position;
        // end test code
        // ray = light.position.xyz - position;
    } else if (light.Type == LIGHT_TYPE_DIRECTIONAL) {
        // directional light
        ray = -light.Direction.xyz;
    }
    return normalize(ray);
}


// trace shadow ray
void TraceShadowRay(GBufferContext gbuffer, inout RayContext rayContext, uint lightIndex, inout RayPayload payload) 
{
    // get ray direction
    float3 direction = GetShadowRaySample(GetLightData(lightIndex), gbuffer.WorldSpacePosition, rayContext.Seed);
    RayDesc ray;
    ray.Origin = gbuffer.WorldSpacePosition;
    ray.Direction = normalize(direction);
    // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
    // TMin should be kept small to prevent missing geometry at close contact areas.
    ray.TMin = 0.005;
    ray.TMax = 10000.0;
    const uint RayFlags = RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER;
    TraceRay(Scene, RayFlags, ~0, 1, 1, 1, ray, payload);
}



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
    // get all lights affecting current pixel
    LightIndics lights = GetLights(gbuffer, CulledLights);

    if (lights.numLights == 0) {
        // no lights
        RenderTarget[DispatchRaysIndex().xy] = float4(0, 0, 0, 1);
    } else {
        // total lighting color
        float4 accumulated = float4(0, 0, 0, 0);
        // ray context
        RayContext ray;
        ray.Seed = RandInit(linearIndex, gFrameNumber);

        // get lighting for each light
        for(uint i = 0; i < lights.numLights; ++i) {
            // light index
            uint lightIndex = lights.lightIndics[i];
            // do deferred lighting
            float falloff = 1.0f;
            float3 color = GetLighting(gbuffer, lightIndex, falloff);
            if (falloff <= 0.001f) {
                // ignore weak lights
                continue;
            }
            RayPayload payload;
            payload.Color = float4(0, 0, 0, 0);
            // tracy shadow ray
            TraceShadowRay(gbuffer, ray, lightIndex, payload);
            // apply shadow
            accumulated += payload.Color * float4(color, 0);
        }
        RenderTarget[DispatchRaysIndex().xy] = accumulated;
    }
}
    

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

[shader("closesthit")]
void ClosestHit(inout RayPayload payload, in SimpleAttributes attr)
{
    payload.Color = float4(0, 0, 0, 0);
}

[shader("miss")]
void Miss(inout RayPayload payload)
{
    // payload.color = float4(0, 0, 0, 1);
    payload.Color = float4(1, 1, 1, 1);
}

#endif // RAYTRACING_HLSL