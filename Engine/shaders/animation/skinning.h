#ifndef __SKINNING__
#define __SKINNING__


#include "../common/common.h"

// animation use 128 constants buffer, bone transform, update for each object draw
cbuffer ArrayAnime: register(b3)
{
    float4x4  gSkinMatrix[128];
}

/*
    bone transformation in local space
*/

VS_Input_Skinning transform_bone(VS_Input_Skinning vs_input)
{
    // get bone ids
    uint4 bones = uint4(
        vs_input.Bones & 0x000000ff,
        vs_input.Bones >> 8 & 0x000000ff,  
        vs_input.Bones >> 16 & 0x000000ff,        
        vs_input.Bones >> 24      
    );
    // weights
    float4 weights = float4(vs_input.Weight.xyz, 0);
    weights.w = 1.0f - (weights.x + weights.y + weights.z);
    // get matrix
    float4x4 mat0 = gSkinMatrix[bones.x];
    float4x4 mat1 = gSkinMatrix[bones.y];
    float4x4 mat2 = gSkinMatrix[bones.z];
    float4x4 mat3 = gSkinMatrix[bones.w];
    // transform position normal and tangent
    float4 position = float4(vs_input.PosL, 1.0f);
    float4 normal = float4(vs_input.Normal, 0.0f);
    float4 tangent = float4(vs_input.Tangent.xyz, 0);

    weights = float4(weights.x, weights.y, weights.z, weights.w);
    // transform position
    position = mul(position, mat0) * weights.x + mul(position, mat1) * weights.y 
        + mul(position, mat2) * weights.z + mul(position, mat3) * weights.w;
    // transform noraml
    normal = mul(normal, mat0) * weights.x + mul(normal, mat1) * weights.y 
        + mul(normal, mat2) * weights.z + mul(normal, mat3) * weights.w;
    normal = normalize(normal);
    // transform tangent
    tangent = mul(tangent, mat0) * weights.x + mul(tangent, mat1) * weights.y 
        + mul(tangent, mat2) * weights.z + mul(tangent, mat3) * weights.w;
    tangent = normalize(tangent);
    // return output
    vs_input.PosL = position.xyz;
    vs_input.Normal = normal.xyz;
    vs_input.Tangent = tangent.xyz;
    return vs_input;
}

#endif