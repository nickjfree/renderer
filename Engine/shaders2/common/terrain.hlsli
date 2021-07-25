#ifndef __TERRAIN__
#define __TERRAIN__


/*
    vertex
*/
struct Vertex
{
    float3 PosL;
    float3 Normal;
    float2 TexCoord;
    float  InstanceID;
    float3 Tangent;
};



// terrain texture
Texture2D gTerrainVirtualTexture : register(t0);

// uav deformable buffer. transformed vertext stored in this buffer
RWStructuredBuffer<Vertex> gDeformableBuffer     : register(u0);

/*
    snap terrain vertex to grid
*/
float3 snap_terrain_vertex(float3 position, float3 cameraPosition) 
{
    // get climap level
    float sampleLevel = floor(position.y / 2 + 0.5);
    int snapLevel =  floor((position.y /2) + 0.5);
    int snapUnit = 1 << snapLevel;
    float invertUnit = 1.0 / snapUnit;
    // snap vertex to grid
    // cameraPosition = float3(0.9, 0, 0.9);
    position += 0.5 * float3(snapUnit, 0, snapUnit);
    float3 centerPosition = cameraPosition;//  + snapUnit * 0.5 + ;
    centerPosition = floor(centerPosition * invertUnit) * snapUnit ;
    return float3(position.x + centerPosition.x, sampleLevel, position.z + centerPosition.z);
}

/*
    get vertex height, sample the virtual texture
*/
float get_terrain_vertex_height(float2 uv, int level)
{
    return 0;
}


/*
    get virtual texture uv
*/
float2 get_virtual_texture_uv(float3 position, int level)
{
    float2 tileOffset = float2(fmod(level, 4.0), floor(level / 4.0)) * float2(0.25f, 0.5f);
    float scale = (1 << level) * 64;
    float2 subOffset = (floor(fmod(position.xz / scale, 2.0)) + frac(position.xz/scale)) * float2(0.125f, 0.25f);
    float2 uv =  tileOffset + subOffset;
    uv.y = 1 - uv.y;
    return uv;
}


/*
    terrain transformation 
*/
Vertex transform_terrain(Vertex vsInput, uint vertexId)
{
    // get camera position
    float3 cameraPosition = gViewPoint.xyz;
    // transform position normal and tangent
    float3 position = snap_terrain_vertex(vsInput.PosL.xyz, cameraPosition);
    // sample level
    int level = (float)position.y;
    // calc uv
    float2 uv = get_virtual_texture_uv(position, level); //position.xz / 8.0f;
    // sample height map
    position.y = get_terrain_vertex_height(uv, level);
    // noraml is always up
    float4 normal = float4(0, 1, 0, 0);
    // transform tangent
    float3 tangent = vsInput.Tangent;
    // return output
    vsInput.PosL = position.xyz;
    vsInput.TexCoord = uv;
    vsInput.Normal = normal.xyz;
    vsInput.Tangent = tangent;

    // store transformed terrain mesh
    gDeformableBuffer[vertexId] = vsInput;

    return vsInput;
}

#endif