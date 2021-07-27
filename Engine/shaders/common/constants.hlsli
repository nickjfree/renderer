#ifndef __CONSTANTS__
#define __CONSTANTS__


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

// samplers
SamplerState gSam : register(s0);
SamplerState gSamBilinear :register(s1);
SamplerState gSamPoint :register(s2);

#endif
