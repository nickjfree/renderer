#include "../common/shader_inputs.h"
#include "../common/gi/probe.hlsli"


RWTexture2D<float4> IrradianceBuffer : register(u0);
RWTexture2D<float4> Output : register(u1);


[numthreads(8, 4, 1)]
void CSMain(uint3 threadId : SV_DispatchThreadID)
{

	int probeIndex = GetProbeIndex(threadId.xy);
	int backfaceCount = 0;
	for (int i = 0; i < CBGIVolume.numRaysPerProbe; ++i) {
		float hitDistance = IrradianceBuffer[int2(i, probeIndex)].a;
		if (hitDistance < 0) {
			backfaceCount += 1;
		}
	}
	if ((float)backfaceCount / CBGIVolume.numRaysPerProbe > CBGIVolume.backfaceThreshold) { 
		Output[threadId.xy] = float4(PROBE_STATE_INACTIVE, 0, 0, 0);
	} else {
		Output[threadId.xy] = float4(PROBE_STATE_ACTIVE, 0, 0, 0);
	}
}
