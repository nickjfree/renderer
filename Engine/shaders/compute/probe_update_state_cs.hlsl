#include "../common/shader_inputs.h"
#include "../common/gi/probe.hlsli"


RWTexture2D<float4> IrradianceBuffer : register(u0);
RWTexture2D<float4> Output : register(u1);


float3 getRayDirection(int i) {
	float3 r = SphericalFibonacci(i, CBGIVolume.numRaysPerProbe);
	return normalize(mul(float4(r, 0), CBGIVolume.rayRotation).xyz);
}

[numthreads(8, 4, 1)]
void CSMain(uint3 threadId : SV_DispatchThreadID)
{

	int probeIndex = GetProbeIndex(threadId.xy);
	int backfaceCount = 0;
	// init distance
	float closestFrontfaceDistance = 1e27;
	float closestBackfaceDistance = 1e27;
	float farthestFrontfaceDistance = 0;
	float farthestBackfaceDistance = 0;

	int closestBackfaceIndex = -1;
    int closestFrontfaceIndex = -1;
    int farthestFrontfaceIndex = -1;

	for (int i = 0; i < CBGIVolume.numRaysPerProbe; ++i) {
		float hitDistance = IrradianceBuffer[int2(i, probeIndex)].a;
		if (hitDistance < 0) {
			// backface hit
			backfaceCount += 1;
			// make distance back to full length
			hitDistance = -hitDistance * 5.0f;

			if (closestBackfaceDistance > hitDistance) {
				closestBackfaceIndex = i;
				closestBackfaceDistance = hitDistance;
			}
			farthestBackfaceDistance = max(farthestBackfaceDistance, hitDistance);

		} else {
			// frontface hit
			if (closestFrontfaceDistance > hitDistance) {
				closestFrontfaceDistance = hitDistance;
				closestFrontfaceIndex = i;
			}
			if (farthestFrontfaceDistance < hitDistance) {
				farthestFrontfaceDistance = hitDistance;
				farthestFrontfaceDistance = i;
			}
		}
	}
	float status = PROBE_STATE_ACTIVE;
	float3 fullOffset = Output[threadId.xy].xyz;
	float3 newOffset = fullOffset;
	// get probe status
	if ((float)backfaceCount / CBGIVolume.numRaysPerProbe > CBGIVolume.backfaceThreshold) {
		// set probe inactive 
		status = PROBE_STATE_INACTIVE;
	}

	if (closestBackfaceIndex  != -1 && (float)backfaceCount / CBGIVolume.numRaysPerProbe > CBGIVolume.backfaceThreshold) {
		// move probe outside
		float3 closestBackfaceDirection = getRayDirection(closestBackfaceIndex);

		float3 offset = closestBackfaceDirection * (closestBackfaceDistance + CBGIVolume.minFrontfaceDistance * 0.5);
		fullOffset += offset;
	} else if (closestFrontfaceDistance < CBGIVolume.minFrontfaceDistance) {
		// to close to frontface, move it away
		float3 farthestFrontfaceDirection = getRayDirection(farthestFrontfaceIndex);
		float3 closestFrontfaceDirection = getRayDirection(closestFrontfaceIndex);

		if (dot(closestFrontfaceDirection, farthestFrontfaceDirection) < 0) {
			float3 offset = farthestFrontfaceDirection * min(1, farthestFrontfaceDistance - CBGIVolume.minFrontfaceDistance);
			fullOffset += offset;
		}
		status = 0.1;
	} else if (closestFrontfaceDistance > CBGIVolume.minFrontfaceDistance) {
		// probe isn't near anythong, move it back
		float moveBackMargin = min(closestFrontfaceDistance - CBGIVolume.minFrontfaceDistance, length(fullOffset));
        float3 moveBackDirection = normalize(-fullOffset);
        fullOffset += (moveBackMargin * moveBackDirection);
        status = 0.2;
	}
	// limit offset
	float3 normalizedOffset = fullOffset / CBGIVolume.probeGridSpacing;
    if (dot(normalizedOffset, normalizedOffset) < 0.2025f) // 0.45 * 0.45 == 0.2025
    {
        newOffset = fullOffset;
    }

	Output[threadId.xy] = float4(newOffset, status);
	// Output[threadId.xy] = float4(0, 0, 0, status);
}
