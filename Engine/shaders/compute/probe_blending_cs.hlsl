#include "../common/shader_inputs.h"
#include "../common/gi/probe.hlsli"


#define RAYS_PER_PROBE 144


#ifdef BLEND_IRRADIANCE
#define THREAD_COUNT 8
groupshared  float3 irradianceData[RAYS_PER_PROBE];
#else
#define THREAD_COUNT 16
#endif

groupshared  float distanceData[RAYS_PER_PROBE];
groupshared  float3 rayDirections[RAYS_PER_PROBE];


RWTexture2D<float4> IrradianceBuffer : register(u0);
RWTexture2D<float4> Output : register(u1);


[numthreads(THREAD_COUNT, THREAD_COUNT, 1)]
void CSMain(uint3 groupId : SV_GroupId, uint3 threadId : SV_GroupThreadID)
{

	uint probeIndex = groupId.x;

	// copy irrandiance data to shared memory
	uint numStep = THREAD_COUNT * THREAD_COUNT;

	uint linearId = threadId.y * THREAD_COUNT + threadId.x; 	
	for (int i = linearId; i < RAYS_PER_PROBE; i += numStep) {

#ifdef BLEND_IRRADIANCE
		// cache irradiance
		irradianceData[i] = IrradianceBuffer[int2(i, probeIndex)].rgb;
#endif
		// cache distance
		distanceData[i] = IrradianceBuffer[int2(i, probeIndex)].a;
		// cache direction
		float3 r = SphericalFibonacci(i, RAYS_PER_PROBE);
		rayDirections[i] = mul(float4(r, 0), CBGIVolume.rayRotation).xyz;
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

		float dist = distanceData[i];

		// ignore backface		
		if (dist < 0.f)
        {
        	// Output[targetCoord.xy] = float4(1, 0, 0, 0);
        	// return;
            continue;
        }

        float3 rayDirection = rayDirections[i];
		float weight = max(0, dot(otcaDirection, rayDirection));

#ifdef BLEND_IRRADIANCE
		float3 irradiance = irradianceData[i];
		result += float4(irradiance * weight, weight);
#else
		// filter distance
	    float maxDistance = length(CBGIVolume.probeGridSpacing) * 0.75f;
		dist = min(abs(dist), maxDistance);
		weight = pow(weight, CBGIVolume.distanceExponent);
		result += float4(dist * weight, dist * dist * weight, 0, weight);

#endif

	}
#ifdef BLEND_IRRADIANCE
	result.xyz *= 1.0f / max(0.001, 2.0f * result.w);

	// smoth the irradiance change
	float3 delta = (result.xyz - previous.xyz);
    if (length(delta) > CBGIVolume.brightnessThreshold)
    {
        result.xyz = previous.xyz + (delta * 0.25f);
    }

	float4 output = float4(lerp(result.xyz, previous.xyz, hysteresis), 1);
#else
	result.xyz *= 1.0f / max(0.001, 2.0f * result.w);
	float4 output = float4(lerp(result.xyz, previous.xyz, hysteresis), 1);
#endif
	Output[targetCoord.xy] = output;
}
