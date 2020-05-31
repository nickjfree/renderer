#ifndef __RAYTRACING__
#define __RAYTRACING__


#include "deferred.h"


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

#endif