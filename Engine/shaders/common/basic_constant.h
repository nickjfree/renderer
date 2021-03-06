#ifndef __BASIC_CONSTANT__
#define __BASIC_CONSTANT__

/*
    basic constant buffer definition
*/


cbuffer PerObject: register(b4)
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

cbuffer PerLight: register(b1)
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

#endif