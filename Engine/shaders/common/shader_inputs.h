#ifndef __CONSTANTS__
#define __CONSTANTS__

#include "shader_types.h"

// field of view
static const float FoV = tan(0.15 * 3.141592654);

/*
    basic constant buffer definition
*/

CBUFFER(PerObject, 0)
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


CBUFFER(PerFrame, 1) 
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
CBUFFER(ArrayBSParamaters, 2)
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
CBUFFER(ArrayKeyframe, 3)
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
CBUFFER(PerLight, 0)
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
CBUFFER(ArraylightInfos, 0)
{
    LightData gLights[256];
}


/*
*    light culling infos
*/
CBUFFER(ArrayLightCullingInfos, 0)
{
    uint numLights;
    uint lightsPerCell;
    uint cellScale;
    uint cellCount;
    float4 lights[256];
}



// fsr
CBUFFER(FSRConstant, 0)
{
    uint4 Const0;
    uint4 Const1;
    uint4 Const2;
    uint4 Const3;
    uint4 Sample;
};


// samplers
SAMPLERSTATE(gSam, 0);
SAMPLERSTATE(gSamBilinear, 1);
SAMPLERSTATE(gSamPoint, 2);


#endif
