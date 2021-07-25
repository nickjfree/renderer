#ifndef __LIGHT_CULLING__
#define __LIGHT_CULLING__

#include "../common/constants.hlsli"


cbuffer ArraylightInfos: register(b0)
{
	uint numLights;
	uint lightsPerCell;
	uint cellScale;
	uint cellCount;
    float4 lights[256];
}


struct LightIndics
{
    uint numLights;
    uint lightIndics[15];
};


#define MAX_LIGHT_COUNT_PER_CELL  16
#define THREAD_COUNT_PER_GROUP    32

RWStructuredBuffer<LightIndics> CulledLights : register(u0);


groupshared uint lightCount;

void ComputeAABB(out float3 Min, out float3 Max, uint3 groupId)
{

	const uint offset = cellCount / 2;
	float3 center = groupId - float3(offset, offset, offset) + 0.5f;

	// determine which sector
	float3 signValue = sign(center);

	// compute bounds as logarithmically increasing from center (with minimum cell size of 2)
	Min = abs(floor(center));
	Max = abs(ceil(center));
	Min = Min > 0.0 ? pow(2.0, Min + 1.0) : 0.0;
	Max = Max > 0.0 ? pow(2.0, Max + 1.0) : 0.0;

	// Extend range of last cell
	Max = (groupId == 0 || groupId == cellCount - 1) ? 1.0e7 : Max;

	// return to sector
	Min *= signValue * 1;//cellScale;
	Max *= signValue * 1;//cellScale;
}


float SquaredDistanceAABBToPoint(float3 center, float3 extents, float3 position)
{
	float3 d = max(abs(position - center) - extents, 0);
	return dot(d, d);
}


[numthreads(THREAD_COUNT_PER_GROUP, 1, 1)]
void CSMain( uint3 groupId : SV_GroupId, uint3 threadId : SV_GroupThreadID)
{

	// init cache data
	if (threadId.x == 0) {
		lightCount = 0;
	}
	// get light index buffer offset
	uint bufferOffset = (groupId.x + cellCount * (groupId.y + (groupId.z * cellCount)));
	// clear all the light index
	if (threadId.x < MAX_LIGHT_COUNT_PER_CELL - 1) {
		CulledLights[bufferOffset].lightIndics[threadId.x] = 0;
	}

	GroupMemoryBarrierWithGroupSync();

	// get AABB by groupId
	float3 Min, Max;
	ComputeAABB(Min, Max, groupId);
	float3 cellCenter = (Min + Max) * 0.5 + gViewPoint.xyz;
	float3 extents = (Max - Min) * 0.5;

	// cull the lights
	for(uint i = threadId.x; i < numLights; i += THREAD_COUNT_PER_GROUP) {
		// just let all the lights pass the culling test for now
		float4 lightDesc = lights[i];
		// position: xyz radius: w
		if (lightDesc.w < 0 || SquaredDistanceAABBToPoint(cellCenter, extents, lightDesc.xyz) < lightDesc.w * lightDesc.w) {
			// light sphere intersect with the AABB
			uint currentIndex;
			InterlockedAdd(lightCount, 1, currentIndex);
			if (currentIndex < MAX_LIGHT_COUNT_PER_CELL - 1) {
				CulledLights[bufferOffset].lightIndics[currentIndex] = i;
			}
		}
	}

	GroupMemoryBarrierWithGroupSync();

	if (threadId.x == 0) {
		CulledLights[bufferOffset].numLights = lightCount;
	}
}

#endif