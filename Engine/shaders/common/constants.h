#ifndef __CONSTANTS__
#define __CONSTANTS__

#include "shader_types.h"

// field of view
static const float FoV = tan(0.15 * 3.141592654);

/*
    basic constant buffer definition
*/

cbuffer PerObject: register(b0)
{
    // object
    float4x4 gWorldViewProjection;
    float4x4 gWorldViewMatrix;
    // prev
    float4x4 gPrevWorldViewProjection;    
    // material
    float gSpecular;
    // object id
    int   gObjectId;
    // pad
    float2 pad_1;
}


cbuffer PerFrame: register(b2) 
{
    // per frame
    float4x4 gViewProjectionMatrix;
    float4x4 gViewMatrix;
    float4x4 gInvertViewMaxtrix;
    float4   gViewPoint;

    int  gTimeElapse;
    int  gAbsoluteTime;
    int  gFrameNumber;
    int  pad;

    // screen size
    float4 gScreenSize;
    float4x4 pad1;

    // post pass 
    float4   gSampleOffsets[4];
    float4   gSampleWeights[4];
}


/*
     blendshape parameters
*/
cbuffer ArrayBSParamaters: register(b3)
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
}

// animation use 128 constants buffer, bone transform, update for each object draw
cbuffer ArrayKeyframe: register(b4)
{
    float4x4  gSkinMatrix[128];
}

// ArrayBSParamaters and ArrayKeyframe use deffirent constant buffers registers because these 2 may be used together




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
cbuffer PerLight: register(b0)
{
    // shadow map constant
    float4x4  gLightViewProjection;
    float4x4  gLightProjection;
    // lighting
    float4 gLightPosition;
    float4 gLightColor;
    float4 gRadiusIntensity;
    float4 gLightDirection;
}

/*
    light constant buffer multi lights
*/
cbuffer ArraylightInfos: register(b0, space0)
{
    LightData gLights[256];
}


/*
*    light culling infos
*/
cbuffer ArrayLightCullingInfos: register(b0)
{
    uint numLights;
    uint lightsPerCell;
    uint cellScale;
    uint cellCount;
    float4 lights[256];
}



// fsr
cbuffer cb : register(b0)
{
    uint4 Const0;
    uint4 Const1;
    uint4 Const2;
    uint4 Const3;
    uint4 Sample;
};


// samplers
SamplerState gSam : register(s0);
SamplerState gSamBilinear :register(s1);
SamplerState gSamPoint :register(s2);

#endif
