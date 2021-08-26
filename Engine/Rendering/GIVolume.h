#ifndef __GI_VOLUME__
#define __GI_VOLUME__


#include "Node.h"


//float3      origin;
//int         numRaysPerProbe;
//float3      probeGridSpacing;
//float       probeMaxRayDistance;
//int3        probeGridCounts;
//int         probeNumIrradianceTexels;
//int         probeNumDistanceTexels;
//float       normalBias;
//float       viewBias;
//float       giPad;
//float4x4    rayRotation;

constexpr int gi_volume_probe_spacing = 1;
constexpr int gi_volume_probe_num_rays = 144;
constexpr int gi_volume_probe_ray_distance = 1000;
constexpr int gi_volume_probe_irradiance_texels = 8;
constexpr int gi_volume_probe_distance_texels = 16;
constexpr float gi_volume_normal_bias = 0.01f;
constexpr float gi_volume_view_bias = 0.01f;


class GIVolume: public Node
{
	DECLARE_ALLOCATER(GIVolume);
public:
	GIVolume();
	// queue render command
	virtual int Render(CommandBuffer* cmdBuffer, int stage, int lod, RenderingCamera* camera, RenderContext* renderContext);
	// set position
	void SetPosition(Vector3& Position_);
	// set volume scale
	void SetScale(Vector3 scale);

private:
	// volume describe
	CBGIVolume giVolume;
	// num probe 
	int numProbes = 0;
	// probe textures
	int irradianceBuffer = -1;
	int distanceBuffer = -1;
	int irradianceMap = -1;
	int distanceMap = -1;
	int stateMap = -1;
	// lights
	int culledLights = -1;
	CBLights* cbLights = nullptr;
};


#endif

