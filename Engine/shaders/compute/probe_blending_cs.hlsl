#include "../common/shader_inputs.h"
#include "../common/gi/probe.hlsli"


#define RAYS_PER_PROBE 144


#ifdef BLEND_IRRANDIANCE
#define THREAD_COUNT 8
groupshared  float3 irrandianceData[RAYS_PER_PROBE];
RWTexture2D<float4> Output : register(u1);	
#else
#define THREAD_COUNT 16
groupshared  float irrandianceData[RAYS_PER_PROBE];
RWTexture2D<float4> Output : register(u1);	
#endif




RWTexture2D<float4> IrrandianceBuffer : register(u0);

// debug
RWTexture2D<float4> Output2 : register(u2);


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
	int2 probeBase = GetMapBaseCoord(probeIndex);
	int2 targetCoord = probeBase + int2(1, 1) + threadId.xy;

	float4 result = float4(0, 0, 0, 0);

	float  hysteresis = CBGIVolume.hysteresis;
    float4 previous = Output[targetCoord.xy];

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
#ifdef BLEND_IRRANDIANCE
	result.xyz *= 1.0f / max(0.001, 2.0f * result.w);
	float4 output = float4(lerp(result.xyz, result.xyz, 0.01), 1);
#else
#endif
	Output[targetCoord.xy] = output;
	Output2[targetCoord.xy] = output;
}
