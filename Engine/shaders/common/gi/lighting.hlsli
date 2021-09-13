#ifndef __GI_LIGHTING__
#define __GI_LIGHTING__


#include "probe.hlsli"


// probe irradiance
Texture2D IrrandianceMap : register(t4);
// probe distance
Texture2D DistanceMap : register(t5);
// probe state
Texture2D StateMap : register(t6);


// get gi lighting
float3 GetGIIrradiance(float3 position, float3 normal, float3 bias)
{
	// base probe coord
	int3 probeBase = GetProbeBaseCoord(position);
	// base probe position
	float3 probeBasePosition = GetProbeBasePosition(position);
	float3 shadingPosition = position - probeBasePosition;
	// normalized position
	float3 normalizedPosition = shadingPosition / CBGIVolume.probeGridSpacing;
	// biased position
	float3 biasedPosition = position + bias; 
	// irradiance
	float4 irradiance = float4(0, 0, 0, 0);
	// vlomue weight
	float volumeWeight = GetVolumeWeight(position);
	// get the 8 adjacent probe
	for(uint i = 0; i < 8; ++i) {
		int3 probeOffset = uint3(i, i >> 1, i >>2) & uint3(1, 1, 1);
		// probe coord
		int3 probeCoord = probeBase + probeOffset;
		// to probe distance
		float3 probePosition = probeBasePosition + probeOffset * CBGIVolume.probeGridSpacing;
		float3 probeToPoint = biasedPosition - probePosition;
		float pointDistance = length(probeToPoint);	
		// probe direction
		float3 probeDirection = normalize(probeToPoint);	
		// uv
		float2 uvIrradiance = GetIrrandianceMapUV(probeCoord, normal);
		float2 uvDistance = GetDistanceMapUV(probeCoord, normalize(probeDirection));

		uint probeIndex = GetProbeIndex(probeCoord);
		float probeState = StateMap[GetMapBaseCoord(probeIndex)].x;

		// test
		// if (floor(probePosition.x) != -9) {
		// 	continue;
		// }

		// // if (floor(probePosition.y) != 4) {
		// // 	continue;
		// // }

		// if (floor(probePosition.z) != -2) {
		// 	continue;
		// }


		// if (!(floor(probePosition.y) == 5 || floor(probePosition.y) == 4)) {
		// 	continue;
		// }
		if (probeState != PROBE_STATE_ACTIVE) {
			continue;
		}
		// weight
		float weight = 1.0f;
		// wrap
		float wrapWeight = (dot(-probeDirection, normal) + 1.f) * 0.5f;
		weight *= (wrapWeight * wrapWeight) + 0.2f;
		// blend
		float3 blend = max(0.00001f, lerp(1 - normalizedPosition, normalizedPosition, probeOffset));
		// probe visibility distance
		float2 visDistance = DistanceMap.SampleLevel(gSamBilinear, uvDistance, 0).xy;

		float deltaDistance = pointDistance - visDistance.x;
		if (deltaDistance > 0) {
			// the point is in shadow, decrease the probe weight
			float variance = abs(visDistance.x * visDistance.x - visDistance.y);
			float visWeight = max(0, variance / (deltaDistance * deltaDistance + variance));
			weight *= max((visWeight * visWeight * visWeight), 0);
			// continue;
			// return float3(weight, deltaDistance, variance);
		}
		// return float3(probeDirection);
		const float crushThreshold = 0.2f;
		if (weight < crushThreshold)
		{
			weight *= (weight * weight) * (1.f / (crushThreshold * crushThreshold));
		}
		// blend probes
		weight *= (blend.x * blend.y * blend.z);
		// ensure weight > 0
		weight = max(0, weight);
		// return float3(weight, deltaDistance, visDistance.x);
		// get irradiance
		float4 irr = IrrandianceMap.SampleLevel(gSamBilinear, uvIrradiance, 0);
		// irradiance += 1 * weight;
		irradiance += float4(irr.rgb * weight, weight);
		// irradiance += float4(abs(deltaDistance) * weight, weight);
	}
	if (irradiance.w <= 0) {
		return float3(0, 0, 0);
	}
	irradiance.rgb *= 1.0f / irradiance.w;
	// return float4(probeBase, 0);
	return irradiance.rgb * volumeWeight;
}


#endif