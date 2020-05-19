//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#ifndef RAYTRACINGHLSLCOMPAT_H
#define RAYTRACINGHLSLCOMPAT_H

struct Viewport
{
    float left;
    float top;
    float right;
    float bottom;
};

struct RayGenConstantBuffer
{
    Viewport viewport;
    Viewport stencil;
};


cbuffer g_rayGenCB: register(b0)
{
    float4x4 gInvertViewMaxtrix;
    float4   gViewPoint;
}

static const float fov = tan(0.15 * 3.141592654);

float4 GetLookVector(float2 uv, float aspect)
{
    float2 clip_uv = (uv  - 0.5) * float2(2,-2);
    float fary = fov;
    float farx = aspect * fary;
    float4 LookVec = float4(float2(farx, fary) * clip_uv, 1, 0);
    return normalize(LookVec);
}


#endif // RAYTRACINGHLSLCOMPAT_H