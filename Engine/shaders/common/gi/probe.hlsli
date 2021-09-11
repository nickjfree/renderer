#ifndef __PROBE__
#define __PROBE__


#include "../../common/shader_inputs.h"
#include "operations.hlsli"

#define PROBE_STATE_ACTIVE    0
#define PROBE_STATE_INACTIVE  1


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

uint GetProbeIndex(uint2 XY)
{
	uint probeIndex = XY.x % CBGIVolume.probeGridCounts.x;
	probeIndex += floor(XY.x / CBGIVolume.probeGridCounts.x) * numProbePerPlane();
	probeIndex += XY.y * CBGIVolume.probeGridCounts.x;
	return probeIndex;
}

uint GetProbeIndex(int3 probeCoord)
{
	return probeCoord.x + probeCoord.y * numProbePerPlane() + probeCoord.z * CBGIVolume.probeGridCounts.x;
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
#ifdef BLEND_IRRADIANCE
	return CBGIVolume.probeNumIrradianceTexels;
#else
	return CBGIVolume.probeNumDistanceTexels;
#endif
}

uint GetTexelsWithBorder()
{
#ifdef BLEND_IRRADIANCE
	return CBGIVolume.probeNumIrradianceTexels + 2;
#elif BLEND_DISTANCE
	return CBGIVolume.probeNumDistanceTexels + 2;
#else
	return 1;
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

// get base probe coord by position
int3 GetProbeBaseCoord(float3 position)
{
	float3 volumeBase = CBGIVolume.origin - (0.5 * CBGIVolume.probeGridCounts * CBGIVolume.probeGridSpacing);
	position -= volumeBase;
	position *= 1.0f / CBGIVolume.probeGridSpacing;
	return floor(position);
}

float3 GetProbeBasePosition(float3 position)
{
	int3 coord = GetProbeBaseCoord(position);
	coord -= (0.5 * CBGIVolume.probeGridCounts);
	return coord * CBGIVolume.probeGridSpacing + CBGIVolume.origin; 
}


float GetVolumeWeight(float3 position)
{
	float3 normlizedPos = 2 * (position - CBGIVolume.origin) / (CBGIVolume.probeGridCounts * CBGIVolume.probeGridSpacing);
	float3 weight = max(0, 1 - abs(normlizedPos));
	weight = ceil(weight);
	return weight.x * weight.y * weight.z;
}


// normalized octahedron coord
float2 NormalizedOctaCoord(int2 xy)
{
	uint texels = GetTexelsNoBorder();
	float2 coord = float2(xy.x % texels, xy.y % texels);
	// coord + 0.5 / texels  * 2 - 1
	coord = (coord + 0.5) / texels;
	coord *= 2;
	return  float2(coord.x - 1, 1 - coord.y); // (coord * 2.0f + 1.0f) / texels - float2(1, 1);
}

// normalized octahedron coord to direction
float3 OctaToDirection(float2 uv)
{
	float3 direction = float3(uv, 1 - abs(uv.x) - abs(uv.y));
	if (direction.z < 0) {
		direction.xy = float2(1 - abs(uv.yx)) * sign(uv.xy);
	}
	return normalize(direction);
}

float2 DirectionToOctaUV(float3 direction)
{
	float total = abs(direction.x) + abs(direction.y) + abs(direction.z);
	direction *= 1.0f / total;
	if (direction.z < 0) {
		direction.xy = float2(1 - abs(direction.yx)) * sign(direction.xy);
	}
	return float2(direction.x, -direction.y);
}

float2 GetMapBaseUV(int3 probeCoord)
{
	uint baseX = probeCoord.x + probeCoord.y * CBGIVolume.probeGridCounts.x;
	uint baseY = probeCoord.z;
	uint2 base = uint2(baseX, baseY);
	float width = CBGIVolume.probeGridCounts.x * CBGIVolume.probeGridCounts.y;
	float height = CBGIVolume.probeGridCounts.z;
	float2 baseUV = base / float2(width, height);
	return baseUV;
}

float2 GetIrrandianceMapUV(int3 probeCoord, float3 direction)
{
	float2 baseUV = GetMapBaseUV(probeCoord);
	// get octa uv
	uint texels = CBGIVolume.probeNumIrradianceTexels + 2;
	float width = CBGIVolume.probeGridCounts.x * CBGIVolume.probeGridCounts.y;
	float height = CBGIVolume.probeGridCounts.z;
	float2 texelOffset = 1.0f / (float2(width, height) * texels);
	float2 octaUV = DirectionToOctaUV(direction);
	return baseUV +  texelOffset + (octaUV * 0.5 + 0.5) * CBGIVolume.probeNumIrradianceTexels * texelOffset;
}

float2 GetDistanceMapUV(int3 probeCoord, float3 direction)
{
	float2 baseUV = GetMapBaseUV(probeCoord);
	// get octa uv
	uint texels = CBGIVolume.probeNumDistanceTexels + 2;
	float width = CBGIVolume.probeGridCounts.x * CBGIVolume.probeGridCounts.y;
	float height = CBGIVolume.probeGridCounts.z;
	float2 texelOffset = 1.0f / (float2(width, height) * texels);
	float2 octaUV = DirectionToOctaUV(direction);
	return baseUV + texelOffset + (octaUV * 0.5 + 0.5) * CBGIVolume.probeNumDistanceTexels * texelOffset;
}

#endif