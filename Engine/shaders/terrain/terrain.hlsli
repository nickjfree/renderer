#ifndef __TERRAIN__
#define __TERRAIN__


#include "../common/common.h"
#include "../common/deferred.h"


// uav deformable buffer. transformed vertext stored in this buffer
RWStructuredBuffer<VS_Input_Simple> gDeformableBuffer     : register( u0 );

/*
    snap terrain vertex to grid
*/
float3 snap_terrain_vertex(float3 position, float3 cameraPosition) 
{
    // get climap level
    int level = floor(position.y / 2) + 1;
    float levelScale = 1.0/level;
    // snap vertex to grid
    float3 centerOffset = floor(cameraPosition) * levelScale;
    centerOffset = floor(centerOffset) * level;
    return float3(position.x + centerOffset.x, level, position.z + centerOffset.z);
}

/*
    get vertex height, sample the virtual texture
*/
float get_terrain_vertex_height(float2 uv, int level)
{
    return 0;
}


/*
    terrain transformation 
*/
VS_Input_Simple transform_terrain(VS_Input_Simple vs_input)
{
    // get camera position
    float3 cameraPosition = gViewPoint.xyz;
    // transform position normal and tangent
    float3 position = snap_terrain_vertex(vs_input.PosL.xyz, cameraPosition);
    // calc uv
    float2 uv = position.xz / 8.0f;
    // sample height map
    position.y = get_terrain_vertex_height(uv, position.y);
    // noraml is always up
    float4 normal = float4(0, 1, 0, 0);
    // transform tangent
    float4 tangent = float4(1, 0, 0, 0);
    // return output
    vs_input.PosL = position.xyz;
    vs_input.TexCoord = uv;
    vs_input.Normal = normal.xyz;
    vs_input.Tangent = tangent.xyz;

    return vs_input;
}

#endif