#ifndef __RAYTRACING__
#define __RAYTRACING__


#include "../common/constants.hlsli"


// the scene
RaytracingAccelerationStructure Scene : register(t0, space0);


// hitgroup mesh buffer
ByteAddressBuffer Vertices : register(t0, space1);
ByteAddressBuffer Indices : register(t1, space1);

// hitgroup materials
Texture2D gDiffuseMap0 : register(t2, space1);
Texture2D gNormalMap0 : register(t3, space1);
Texture2D gSpecularMap0 : register(t4, space1);

// attributes
typedef BuiltInTriangleIntersectionAttributes SimpleAttributes;

// info about the mesh
cbuffer InstanceInfo: register(b0, space1)
{
    // size of the mesh vertex
    uint gVertexStride;
}


// hitpoint
struct HitPointContext
{
    // world space
    float3 WorldSpacePosition;
    float3 WorldSpaceNormal;
    float3 WorldSpaceLookVector;
    // view space
    float3 ViewSpacePosition;
    float3 ViewSpaceNormal;
    float3 ViewSpaceLookVector;
    // screen space
    float2 ScreenUV;
    bool InScreen;
    // texture uv
    float2 UV;
};


struct HitPointMaterial{
      // colors
    float4 Diffuse;
    float  Roughness;
    float  Specular;
    float  Metallic;
    // transparent
    bool IsTransparent;
};


// ray context
struct RayContext
{
    // random seed
    uint Seed;
};

// load vertex position
float3 LoadVertexPositionFloat3(ByteAddressBuffer SourceBuffer, uint Index, uint StrideInBytes)
{
    uint OffsetInBytes = Index * StrideInBytes;
    return asfloat(SourceBuffer.Load3(OffsetInBytes));
}

// load vertex normal
float3 LoadVertexNormalFloat3(ByteAddressBuffer SourceBuffer, uint Index, uint StrideInBytes)
{
    uint OffsetInBytes = Index * StrideInBytes;
    // offset 1 * float3
    return asfloat(SourceBuffer.Load3(OffsetInBytes + 12));
}

// load vertex uv
float2 LoadVertexUVFloat2(ByteAddressBuffer SourceBuffer, uint Index, uint StrideInBytes)
{
    uint OffsetInBytes = Index * StrideInBytes;
    // offset 2 * float3
    return asfloat(SourceBuffer.Load2(OffsetInBytes + 24));
}

// load indices
uint3 LoadIndices16Bit(ByteAddressBuffer SourceBuffer, uint OffsetInBytes)
{
    uint3 Result;

    // ByteAddressBuffer loads must be aligned to DWORD boundary.
    // We can load 2 DWORDs (4 SHORTs) at a time and extract 3 SHORTs (first 3 or second 3).
    uint AlignedOffsetInBytes = OffsetInBytes & (~3);
    const uint2 PackedIndices = SourceBuffer.Load2(AlignedOffsetInBytes);

    if (AlignedOffsetInBytes == OffsetInBytes)
    {
        // Extract first 3 SHORTs from 2 DWORDs
        Result[0] = PackedIndices[0] & 0xffff;
        Result[1] = PackedIndices[0] >> 16;
        Result[2] = PackedIndices[1] & 0xffff;
    }
    else
    {
        // Extract second 3 SHORTs from 2 DWORDs
        Result[0] = PackedIndices[0] >> 16;
        Result[1] = PackedIndices[1] & 0xffff;
        Result[2] = PackedIndices[1] >> 16;
    }
    return Result;
}


// infos about the hit point
HitPointContext GetHitPointContext(SimpleAttributes attr)
{
    // get barycentrics
    float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);
    // PrimitiveIndex
    uint index0 = PrimitiveIndex() * 3;

    // get the triangle indices
    uint3 indices = LoadIndices16Bit(Indices, index0 * 2);
    // get the triangle uvs
    float2 uv0 = LoadVertexUVFloat2(Vertices, indices[0], gVertexStride);
    float2 uv1 = LoadVertexUVFloat2(Vertices, indices[1], gVertexStride);
    float2 uv2 = LoadVertexUVFloat2(Vertices, indices[2], gVertexStride);
    float2 uv = uv0 * barycentrics.x + uv1 * barycentrics.y + uv2 * barycentrics.z;
    
    HitPointContext hitPoint = (HitPointContext)0;
    hitPoint.UV = uv;

    float3 hitPosition = WorldRayOrigin() + RayTCurrent() * WorldRayDirection();

    hitPoint.WorldSpacePosition = hitPosition;
    hitPoint.ViewSpacePosition = mul(float4(hitPosition, 1), gViewMatrix).xyz;

    // get screen space uv
    float4 screenPosition = mul(float4(hitPosition, 1), gViewProjectionMatrix);
    screenPosition.x = screenPosition.x / screenPosition.w * 0.5 + 0.5;
    screenPosition.y = -screenPosition.y / screenPosition.w * 0.5 + 0.5;
    hitPoint.ScreenUV = screenPosition.xy;
    // is hitpoint in screen?
    hitPoint.InScreen = saturate(screenPosition.x) == screenPosition.x && saturate(screenPosition.y) == screenPosition.y;

    return hitPoint;
}


// get hit point material
HitPointMaterial GetHitPointMaterial(HitPointContext hitPoint)
{
    HitPointMaterial material = (HitPointMaterial)0;
    material.Diffuse = gDiffuseMap0.SampleLevel(gSam, hitPoint.UV, 0);

#ifdef TRANSPARENT
    // handle transparent textures
    material.IsTransparent =  material.Diffuse.w < 0.001;
#endif
    return material;
}

#endif