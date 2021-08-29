#ifndef __PROBE__
#define __PROBE__


#include "../../common/shader_inputs.h"


uint numProbePerPlane()
{
	return CBGIVolume.probeGridCounts.x * CBGIVolume.probeGridCounts.z;
}

// get probe coord
float3 GetProbeCoord(int probeIndex)
{
	return float3(
		probeIndex % CBGIVolume.probeGridCounts.x, 
		probeIndex / numProbePerPlane(), 
		(probeIndex / CBGIVolume.probeGridCounts.x) % CBGIVolume.probeGridCounts.z);
}

// get probe position
float3 GetProbePosition(int probeIndex)
{
	float3 coord = GetProbeCoord(probeIndex);
	coord -= (0.5 * CBGIVolume.probeGridCounts);
	return coord * CBGIVolume.probeGridSpacing + CBGIVolume.origin;
}


#endif