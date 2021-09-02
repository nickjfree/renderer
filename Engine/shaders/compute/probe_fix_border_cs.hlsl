#include "../common/shader_inputs.h"
#include "../common/gi/probe.hlsli"



#ifdef BLEND_IRRANDIANCE
#define THREAD_COUNT 32
#else
#define THREAD_COUNT 64
#endif



RWTexture2D<float4> Input : register(u0);

static const uint borderLen = THREAD_COUNT >> 2;
static const uint indexMax = (THREAD_COUNT >> 2) - 1;
static const int2 borderStart[4] = { int2(0, 0), int2(indexMax, 0), int2(indexMax, indexMax), int2(0, indexMax) };
static const int2 borderSteps[4] = {  int2(1, 0), int2(0, 1), int2(-1, 0), int2(0, -1) };
static const int2 dstOffset[4] = { int2(0, -1), int2(1, 0), int2(0, 1), int2(-1, 0) };


[numthreads(THREAD_COUNT, 1, 1)]
void CSMain(uint3 groupId : SV_GroupId, uint3 threadId : SV_GroupThreadID)
{

	uint borderIndex = threadId.x / borderLen;
	uint pixelIndex = threadId.x % borderLen;
	// fix border
	int2 borderStep = borderSteps[borderIndex];
	int2 len = borderStep * pixelIndex;
	int2 srcCoord = borderStart[borderIndex] + len;
	int2 dstCoord = borderStart[( borderIndex + 1) % 4] - len;
	dstCoord += dstOffset[borderIndex];

	// move to global coord
	srcCoord += groupId.xy * (borderLen + 2) + int2(1, 1);
	dstCoord += groupId.xy * (borderLen + 2) + int2(1, 1);

	Input[dstCoord.xy] = Input[srcCoord.xy];

	// fix cornor
	if (pixelIndex == 0) {
		uint2 dstCoord = borderStart[(borderIndex + 2) % 4];
		dstCoord += dstOffset[( borderIndex + 1) % 4] + borderSteps[( borderIndex + 1) % 4];
		// move to global coord
		dstCoord += groupId.xy * (borderLen + 2) + uint2(1, 1);
		Input[dstCoord.xy] = Input[srcCoord.xy];
	}
}
