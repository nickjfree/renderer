#ifndef __GI_LIGHTING__
#define __GI_LIGHTING__


#include "probe.hlsli"


// probe irradiance
Texture2D IrrandianceMap : register(t4);
// probe distance
Texture2D DistanceMap : register(t5);


// get gi lighting
float3 GetGIIrradiance(float3 position, float3 normal)
{
	// base probe coord
	int3 probeBase = GetProbeBaseCoord(position);
	// base probe position
	float3 probeBasePosition = GetProbeBasePosition(position);
	float3 shadingPosition = position - probeBasePosition;
	// normalized position
	float3 normalizedPosition = shadingPosition / CBGIVolume.probeGridSpacing;

	float4 irradiance = float4(0, 0, 0, 0);

	float volumeWeight = GetVolumeWeight(position);

	// get the adjacent 8 probe
	for(uint i = 0; i < 8; ++i) {
		int3 probeOffset = uint3(i, i >> 1, i >>2) & uint3(1, 1, 1);
		// probe coord
		int3 probeCoord = probeBase + probeOffset;
		// uv
		float2 uvIrradiance = GetIrrandianceMapUV(probeCoord, normal);
		float2 uvDistance = GetDistanceMapUV(probeCoord, normal);
		// weight
		float weight = 1.0f;
		// blend
		float3 blend = max(0.00001f, lerp(1 - normalizedPosition, normalizedPosition, probeOffset));
		weight *= (blend.x * blend.y * blend.z);
		weight = max(0.00001f, weight);
		// get irradiance
		float4 irr = IrrandianceMap.SampleLevel(gSamBilinear, uvIrradiance, 0);
		// irradiance += 1 * weight;
		irradiance += float4(irr.rgb * weight, weight);
	}
	irradiance.rgb /= max(0.000001f, irradiance.w);
	// return float4(probeBase, 0);
	return irradiance.rgb * volumeWeight;
}


#endif