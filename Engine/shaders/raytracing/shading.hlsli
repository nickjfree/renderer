#ifndef __SHADING__
#define __SHADING__


#include "raytracing.hlsli"
#include "random.hlsli"
#include "sampling.hlsli"
#include "../common/lighting.hlsli"
#include "../common/gi/lighting.hlsli"

// culled lights
StructuredBuffer<LightIndics> CulledLights : register(t1, space0);

struct ShadowRayPayload
{
	// color
	float4 Color;        // 4
};

struct ShadingRayPayload
{
	// color
	float3 Position;      
	float3 Normal;
	float3 LookVector;
	float4 Diffuse;
	float3 Specular;
	float Roughness;
	float Metallic;
	float HitDistance;
	bool Hit;
};   // 80

GBufferContext ShadingPayloadToGBufferContext(ShadingRayPayload payload)
{

	GBufferContext gbuffer = (GBufferContext)0;

	gbuffer.WorldSpacePosition = payload.Position;
	gbuffer.ViewSpacePosition = mul(float4(payload.Position, 1.0f), gViewMatrix).xyz;

	gbuffer.WorldSpaceNormal = payload.Normal;
	gbuffer.ViewSpaceNormal = mul(float4(payload.Normal, 0.0f), gViewMatrix).xyz;


	// look vector is ray direction
	gbuffer.WorldSpaceLookVector = payload.LookVector;
	gbuffer.ViewSpaceLookVector = mul(float4(payload.LookVector, 0.0f), gViewMatrix).xyz;

	gbuffer.Diffuse = payload.Diffuse;
	gbuffer.Specular = payload.Specular;
	gbuffer.Roughness = payload.Roughness;
	gbuffer.Metallic = payload.Metallic;
	return gbuffer;
}

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

void TraceShadingRay(float3 origin, float3 direction, inout ShadingRayPayload payload)
{
	// get ray
	RayDesc ray;
	ray.Origin = origin;
	ray.Direction = direction;
	// Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
	// TMin should be kept small to prevent missing geometry at close contact areas.
	ray.TMin = 0.005;
	ray.TMax = 10000.0;
	TraceRay(Scene, RAY_FLAG_FORCE_OPAQUE, ~0, 0, 1, 0, ray, payload);
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

void TraceReflectionRay(GBufferContext gbuffer, inout RayContext rayContext, inout ShadingRayPayload payload, inout float NoL)
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

	NoL = saturate(dot(rayDir, normal));
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
			// color = 0;
			if (falloff <= 0.001f || dot(color, color) <= 0.001f ) {
				// ignore weak lights
				continue;
			}
			ShadowRayPayload payload;
			payload.Color = float4(0, 0, 0, 0);
			// tracy shadow ray
			TraceShadowRay(gbuffer, ray, lightIndex, payload);
			// apply shadow
			accumulated += payload.Color * float4(color, 0);
		}
		return accumulated;
	}
}


float4 ComputeReflectionLighting(GBufferContext gbuffer, RayContext ray)
{
	ShadingRayPayload payload = (ShadingRayPayload)0;
	float NoL = 0.0f;
	TraceReflectionRay(gbuffer, ray, payload, NoL);
	// get lighting for the hit point
	if (!payload.Hit) {
		// not hit
		return payload.Diffuse * NoL;
	} else {
		GBufferContext hit = ShadingPayloadToGBufferContext(payload);
		return ComputeDirectLighting(hit, ray) * NoL;
	}

}

float4 ComputeIndirectLighting(GBufferContext gbuffer)
{
	float3 position = gbuffer.WorldSpacePosition;
	float3 normal = gbuffer.WorldSpaceNormal;
	// get irradiance
	float3 bias = CBGIVolume.normalBias * normal + CBGIVolume.viewBias * gbuffer.WorldSpaceLookVector;
	float3 irradiance = GetGIIrradiance(position, normal, bias);
	// diffuse lighting
	float3 diffuse = irradiance * gbuffer.Diffuse.rgb;
	// no diffuse for metalic materials
	diffuse = lerp(diffuse, 0, gbuffer.Metallic);
	return float4(diffuse, 0);
}

float4 ComputeGIProbeTracingRadiance(float3 origin, float3 direction, RayContext ray)
{
	ShadingRayPayload payload = (ShadingRayPayload)0;
	TraceShadingRay(origin, direction, payload);
	if (!payload.Hit) {
		// not hit
		return float4(payload.Diffuse.xyz, 1e9);
	} else {
		GBufferContext hit = ShadingPayloadToGBufferContext(payload);
		float4 direct = ComputeDirectLighting(hit, ray);
		// multi bounce
		float4 indirect = ComputeIndirectLighting(hit);
		// indirect = 0;
		// indirect = 0;
		return float4(direct.xyz + indirect.xyz * 0.8, payload.HitDistance);
	}
}

#endif