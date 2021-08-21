#ifndef __SHADING__
#define __SHADING__


#include "raytracing.hlsli"
#include "random.hlsli"
#include "sampling.hlsli"
#include "../common/lighting.hlsli"

// culled lights
StructuredBuffer<LightIndics> CulledLights : register(t1, space0);

struct ShadowRayPayload
{
    // color
    float4 Color;
};

struct ReflectionRayPayload
{
    // color
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
void TraceShadowRay(GBufferContext gbuffer, inout RayContext rayContext, uint lightIndex, inout ShadowRayPayload payload) 
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

void TraceReflectionRay(GBufferContext gbuffer, inout RayContext rayContext, inout ReflectionRayPayload payload)
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

    float NoL = saturate(dot(rayDir, normal));
    // get ray
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = normalize(rayDir);
    // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
    // TMin should be kept small to prevent missing geometry at close contact areas.
    ray.TMin = 0.005;
    ray.TMax = 10000.0;
    TraceRay(Scene, RAY_FLAG_FORCE_OPAQUE, ~0, 0, 1, 0, ray, payload);
    payload.Color = payload.Color * NoL;
}

float4 ComputeDirectLighting(GBufferContext gbuffer, RayContext ray)
{
	float4 accumulated = float4(0, 0, 0, 0);
	// get all lights affecting current pixel
    LightIndics lights = GetLights(gbuffer, CulledLights);

    if (lights.numLights == 0) {
	    return accumulated;
    } else {
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
	        ShadowRayPayload payload;
	        payload.Color = float4(0, 0, 0, 0);
	        // tracy shadow ray
	        TraceShadowRay(gbuffer, ray, lightIndex, payload);
	        // apply shadow
	        accumulated += payload.Color * float4(color, 0);
	        // accumulated += float4(color, 0) * 10;
	    }
	    return accumulated;
	}
}


float4 ComputeReflectionLighting(GBufferContext gbuffer, RayContext ray)
{
	ReflectionRayPayload payload;
	payload.Color = float4(0, 0, 0, 0);
	TraceReflectionRay(gbuffer, ray, payload);
	return payload.Color;
}

#endif