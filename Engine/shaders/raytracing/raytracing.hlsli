#ifndef __RAYTRACING__
#define __RAYTRACING__


#include "../common/deferred.h"
#include "../lighting/lighting.h"

ByteAddressBuffer Vertices : register(t0, space1);
ByteAddressBuffer Indices : register(t1, space1);

Texture2D gDiffuseMap0 : register(t2, space1);
Texture2D gNormalMap0 : register(t3, space1);
Texture2D gSpecularMap0 : register(t4, space1);

typedef BuiltInTriangleIntersectionAttributes SimpleAttributes;

// info about the instance
cbuffer InstanceInfo: register(b0, space1)
{
    // size of the vertex
    uint gVertexStride;
}


struct MaterialData 
{
    float3 position;
    float3 normal;
    float  roughness;
    float  specular;
    float  metallic;
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



float4 SampleHitPointColor(SimpleAttributes attr)
{

    // can't get color from screen space, get it from textures
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

    float4 color = gDiffuseMap0.SampleLevel(gSam, uv, 0);

    return color;
}





#endif