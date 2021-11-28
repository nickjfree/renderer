#ifndef __GI_LIGHTING__
#define __GI_LIGHTING__


#include "probe.hlsli"


// probe irradiance
Texture2D IrrandianceMap : register(t4);
// probe distance
Texture2D DistanceMap : register(t5);
// probe state
Texture2D StateMap : register(t6);



bool IsDebugPosition(float3 position)
{
	return (position.x >= -10 && position.x <= -8  &&
		position.y >= 0 && position.y <= 1 &&
		position.z >= -10 && position.z <= -8);
}

bool IsDebugProb1(float3 position)
{
	return floor(position.x) == -10 && floor(position.y) == 0 && floor(position.z) == -10;
}

bool IsDebugProb2(float3 position)
{
	return floor(position.x) == -10 && floor(position.y) == 1 && floor(position.z) == -10;
}

bool IsDebugProb3(float3 position)
{
	return floor(position.x) == -2 && floor(position.y) == 4 && floor(position.z) == 12;
}

bool IsDebugProb4(float3 position)
{
	return floor(position.x) == -2 && floor(position.y) == 4 && floor(position.z) == 14;
}



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
	float3 irradiance = float3(0.0f, 0.0f, 0.0f);
	// vlomue weight
	float volumeWeight = GetVolumeWeight(position);

	int numProbes = 0;
	float accumulatedWeights = 0.0f;
	float3 debugValues = float3(0.0f, 0.0f, 0.0f);
	// get the 8 adjacent probe
	for(uint i = 0; i < 8; ++i) {
		int3 probeOffset = int3(i, i >> 1, i >>2) & int3(1, 1, 1);
		// probe coord
		int3 probeCoord = probeBase + probeOffset;
		// probe state
		uint probeIndex = GetProbeIndex(probeCoord);
		float4 probeState = StateMap[GetMapBaseCoord(probeIndex)];
		// to probe distance
		float3 probePosition = probeBasePosition + probeOffset * CBGIVolume.probeGridSpacing;
		// apply probe relocation
		float3 relocatedProbePosition = probePosition + probeState.xyz;

		float3 probeToPoint = biasedPosition - relocatedProbePosition;
		float pointDistance = length(probeToPoint);	
		// probe direction
		float3 probeDirection = normalize(probeToPoint);	
		// uv
		float2 uvIrradiance = GetIrrandianceMapUV(probeCoord, normal);
		float2 uvDistance = GetDistanceMapUV(probeCoord, normalize(probeDirection));

		// if (!IsDebugProb1(probePosition)) {
		// 	continue;
		// }

		// if (IsDebugProb2(probePosition)) {
		// 	return relocatedProbePosition;
		// }

		if (probeState.a == PROBE_STATE_INACTIVE) {
			continue;
		}
		// weight
		float weight = 1.0f;
		// wrap
		float wrapWeight = (dot(-probeDirection, normal) + 1.f) * 0.5f;
		weight *= (wrapWeight * wrapWeight) + 0.2f;
		// blend
		float3 blend = max(0.001f, lerp(1 - normalizedPosition, normalizedPosition, probeOffset));
		// probe visibility distance
		float2 visDistance = DistanceMap.SampleLevel(gSamBilinear, uvDistance, 0).xy;

		float visWeight = 1.0f;
		float deltaDistance = pointDistance - visDistance.x;
		float variance = 0;
		if (deltaDistance > 0) {
			// the point is in shadow, decrease the probe weight
			variance = abs(visDistance.x * visDistance.x - visDistance.y);
			visWeight = variance / (deltaDistance * deltaDistance + variance);
			visWeight = max((visWeight * visWeight * visWeight), 0);
		}
		weight *= max(0.0f, visWeight);

		weight = max(0.000001f, weight);
		// return float3(probeDirection);
		const float crushThreshold = 0.2f;
		if (weight < crushThreshold)
		{
			weight *= (weight * weight) * (1.f / (crushThreshold * crushThreshold));
		}
		// blend probes
		weight *= (blend.x * blend.y * blend.z);
		// get irradiance
		float4 irr = IrrandianceMap.SampleLevel(gSamBilinear, uvIrradiance, 0);
		// irradiance += 1 * weight;
		irradiance += irr.rgb * weight;
		accumulatedWeights += weight;
		numProbes += 1;

		// if (IsDebugProb1(probePosition)) {
		// 	debugValues = relocatedProbePosition;
		// 	return debugValues;
		// }
		// if (IsDebugProb3(probePosition)) {
		// 	visWeight *= 1000;
		// 	debugValues.x = visDistance.x;
		// 	debugValues.y = deltaDistance;
		// 	debugValues.z = variance;
		// }
		// if (IsDebugProb3(probePosition)) {
		// 	weight *= 1000;
		// 	// // debugValues = probeBasePosition.xyz;
		// 	// debugValues.x = variance;
		// 	debugValues.z = 1;
		// }

	}
	// if (IsDebugPosition(position)) {
	// 	return float3(numProbes, accumulatedWeights, 0);
	// }
	if (accumulatedWeights == 0.0f) {
		return float3(0, 0, 0);
	}
	irradiance.rgb *= (1.0f / accumulatedWeights);
	
	// if (IsDebugPosition(position)) {
	// 	return debugValues;
	// }
	irradiance *= (2 * PI);

	// return float4(probeBase, 0);
	return irradiance.rgb * volumeWeight;
}


#endif