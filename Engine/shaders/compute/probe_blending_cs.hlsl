#include "../common/shader_inputs.h"
#include "../common/gi/probe.hlsli"


#define RAYS_PER_PROBE 144


#ifdef BLEND_IRRANDIANCE
#define THREAD_COUNT 8
groupshared  float3 irrandianceData[RAYS_PER_PROBE];
RWTexture2D<float3> Output : register(u1);	

#else
#define THREAD_COUNT 16
groupshared  float irrandianceData[RAYS_PER_PROBE];
RWTexture2D<float2> Output : register(u1);	

#endif




RWTexture2D<float4> IrrandianceBuffer : register(u0);



[numthreads(THREAD_COUNT, THREAD_COUNT, 1)]
void CSMain(uint3 groupId : SV_GroupId, uint3 threadId : SV_GroupThreadID)
{

	uint probeIndex = groupId.x;

	// copy irrandiance data to shared memory
	uint numStep = THREAD_COUNT * THREAD_COUNT;
	for (int i = 0; i < RAYS_PER_PROBE; i += numStep) {

#ifdef BLEND_IRRANDIANCE
		irrandianceData[i] = IrrandianceBuffer[int2(i, probeIndex)].rgb;
#else
		irrandianceData[i] = IrrandianceBuffer[int2(i, probeIndex)].a;
#endif
	}
	GroupMemoryBarrierWithGroupSync();

	float2 uv = NormalizedOctaCoord(threadId.xy);
	float3 otcaDirection = OctaToDirection(uv);
	uint2 probeBase = GetMapBaseCoord(probeIndex);
	uint2 targetCoord = probeBase + uint2(1, 1) + threadId.xy;

	float4 result = float4(0, 0, 0, 0);

	for (i = 0; i < RAYS_PER_PROBE; ++i) {
		float3 rayDirection = SphericalFibonacci(i, RAYS_PER_PROBE);
		float weight = max(0, dot(otcaDirection, rayDirection));

#ifdef BLEND_IRRANDIANCE
		result += float4(irrandianceData[i] * weight, weight);
#else
		// do some extra ops for weight
		result += float4(irrandianceData[i] * weight, irrandianceData[i] * irrandianceData[i], 0, weight);
#endif
	}
	Output[targetCoord.xy] = result.xyz;
}
