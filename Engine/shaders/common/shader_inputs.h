#ifndef __CONSTANTS__
#define __CONSTANTS__

#include "shader_types.h"
#include "shader_binding_map.h"


// field of view
static const float FoV = (float)tan(0.15 * 3.141592654);

/*
    basic constant buffer definition
*/
CBUFFER(CBInstance, 0)
{
    // object
    float4x4 gWorldViewProjection;
    float4x4 gWorldViewMatrix;
    // prev
    float4x4 gPrevWorldViewProjection;
  
    // object id
    int   gObjectId;
    // pad
    float3 pad_instance;
};


CBUFFER(CBMaterial, 1)
{
    // material
    float gSpecular;
    // pad
    float3 pad_material;
};



CBUFFER(CBFrame, 2)
{

    float4x4 gViewProjectionMatrix;
    float4x4 gViewMatrix;
    float4x4 gInvertViewMaxtrix;

    float4   gViewPoint;
    float4   gScreenSize;
    // time 
    int  gTimeElapse;
    int  gAbsoluteTime;
    int  gFrameNumber;
    int  pad;
   
    float4x4 pad1;
};


/*
*   sample parameters (ugly)
*/
CBUFFER(CBSamplerOffsets, 0)
{
    float4   gSampleOffsets[4];
};

CBUFFER(CBSamplerWeights, 1)
{
    float4   gSampleWeights[4];
};


/*
     blendshape parameters
*/
CBUFFER(CBBlendshape, 3)
{
    // weight array, the first elements contains shape description, followed with weights 
    float4 gWeightsArray[128];
    // gWeightsArray[0].x: shape count
    // gWeightsArray[0].y: shape stride
    // gWeightsArray[0].z: num weights
    // gWeightsArray[0].w: padding
    // gWeightsArray[1]: weights 0
    // gWeightsArray[2]: weights 1
    // gWeightsArray[3]: weights 2
    // .... 
    // gWeightsArray[n]: weights n-1
};

/*
*   bone matrics
*/
CBUFFER(CBSkinningMatrices, 4)
{
    float4x4  gSkinMatrix[128];
};


/*
    light data in constant buffer (raytracing or tiled lighting, world space)
*/
struct LightData
{
    // light color
    float4 Color;
    // light direction
    float4 Direction;
    // light position
    float4 Position;
    // lighting effect radius
    float Radius;
    // intensity
    float Intensity;
    // spotlight inner angle
    float CapsuleDistanceOrInnerAngle; 
    // spotlight outer angle
    float CapsuleRadiusOrOuterAngle;
    // rectlight width
    float RectLightWidth;
    // rectlight height
    float RectLightHeight;
    // light type
    uint Type;
    // pad
    uint Pad;
};

/*
    light constant buffer. one light
*/
CBUFFER(CBLight, 0)
{
    // shadow map constant
    float4x4  gLightViewProjection;
    float4x4  gLightProjection;
    // lighting
    float4 gLightPosition;
    float4 gLightColor;
    float4 gRadiusIntensity;
    float4 gLightDirection;
};

/*
    light constant buffer multi lights
*/
CBUFFER(CBLights, 0)
{
    LightData gLights[256];
};


/*
*    light culling infos
*/
CBUFFER(CBLightsToCull, 0)
{
    uint numLights;
    uint lightsPerCell;
    uint cellScale;
    uint cellCount;
    float4 lights[256];
};


// fsr
CBUFFER(CBFSRConst, 0)
{
    uint4 Const0;
    uint4 Const1;
    uint4 Const2;
    uint4 Const3;
    uint4 Sample;
};

// gi
struct GIVolumeType
{
    float3      origin;
    int         numRaysPerProbe;
    float3      probeGridSpacing;
    float       probeMaxRayDistance;
    int3        probeGridCounts;
    int         probeNumIrradianceTexels;
    int         probeNumDistanceTexels;
    float       normalBias;
    float       viewBias;
    float       giPad;
    float4x4    rayRotation;
};

CONSTANTBUFFER(CBGIVolume, GIVolumeType, 0);

// samplers
SAMPLERSTATE(gSam, 0);
SAMPLERSTATE(gSamBilinear, 1);
SAMPLERSTATE(gSamPoint, 2);


#endif
