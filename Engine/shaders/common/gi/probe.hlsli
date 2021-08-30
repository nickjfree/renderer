#ifndef __PROBE__
#define __PROBE__


#include "../../common/shader_inputs.h"
#include "operations.hlsli"

uint numProbePerPlane()
{
	return CBGIVolume.probeGridCounts.x * CBGIVolume.probeGridCounts.z;
}

// get probe coord
int3 GetProbeCoord(uint probeIndex)
{
	int3 coord = int3(
		probeIndex % CBGIVolume.probeGridCounts.x, 
		probeIndex / numProbePerPlane(), 
		(probeIndex / CBGIVolume.probeGridCounts.x) % CBGIVolume.probeGridCounts.z);
	return coord;
}

// get probe position
float3 GetProbePosition(uint probeIndex)
{
	int3 coord = GetProbeCoord(probeIndex);
	// offset to origin
	coord -= (0.5 * CBGIVolume.probeGridCounts);
	return coord * CBGIVolume.probeGridSpacing + CBGIVolume.origin;
}

uint GetTexelsNoBorder()
{
#ifdef BLEND_IRRANDIANCE
	return CBGIVolume.probeNumIrradianceTexels;
#else
	return CBGIVolume.probeNumDistanceTexels;
#endif
}

uint GetTexelsWithBorder()
{
#ifdef BLEND_IRRANDIANCE
	return CBGIVolume.probeNumIrradianceTexels + 2;
#else
	return CBGIVolume.probeNumDistanceTexels + 2;
#endif
}

// get base coord of irrandiance/distance map 
int2 GetMapBaseCoord(int probeIndex)
{
	int3 probeCoord = GetProbeCoord(probeIndex);
	uint baseX = probeCoord.x + probeCoord.y * CBGIVolume.probeGridCounts.x;
	uint baseY = probeCoord.z;

	return int2(baseX, baseY) * GetTexelsWithBorder();
}

// normalized octahedron coord
float2 NormalizedOctaCoord(int2 xy)
{
	uint texels = GetTexelsNoBorder();
	float2 coord = float2(xy.x % texels, xy.y % texels);
	// coord + 0.5 / texels * 2 - 1
	return (coord * 2.0f + 1.0f) / texels - float2(1, 1);
}

// normalized octahedron coord to direction
float3 OctaToDirection(float2 uv)
{
	float3 direction = float3(uv, 1 - abs(uv.x) + abs(uv.y));
	if (direction.z < 0) {
		direction.xy = float2(1 - abs(uv.yx)) * sign(uv.xy);
	}
	return normalize(direction);
}

#endif