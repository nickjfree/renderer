#ifndef __BASIC_LAYOUT__
#define __BASIC_LAYOUT__

// simple vertex layout
struct VS_Input_Simple
{
    float3 PosL  : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    float  InstanceID : TEXCOORD1;
    float3 Tangent  : TANGENT;
};

// instancing vertex layout
struct VS_Input_Instance
{
    float3 PosL  : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    float  InstanceID : TEXCOORD1;
    float3 Tangent  : TANGENT;
    float4x4 InstanceWVP: InstanceWVP; 
    float4x4 InstanceWV: InstanceWV;
    float4x4 InstancePWVP: InstancePWVP;
    int InstanceObjectId: InstanceObjectId;
};


// skinning(hardware skeletion animation) vertex layout
struct VS_Input_Skinning
{
    float3 PosL  : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    uint  Bones : TEXCOORD1;
    float3 Weight : TEXCOORD2;
    float3 Tangent  : TANGENT;
};


// blendshape
// struct VS_Input_BlendShape
// {
//     float3 PosL  : POSITION;
//     float3 Normal : NORMAL;
//     float2 TexCoord : TEXCOORD;
//     float  InstanceID : TEXCOORD1;
//     float3 Tangent  : TANGENT;
//     uint VertexId : SV_VertexId;
// };

// quad vertex
struct VS_Input_Quad
{
    float3 PosL  : POSITION;
    float2 TexCoord : TEXCOORD;
};


// simple pixel shader input
struct PS_Input_Simple
{
    float4 PosH : SV_POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

// GBuffer pixel shader input
struct PS_Input_GBuffer
{
    float4 PosH : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Normal : NORMAL;
    float4 Tangent : TEXCOORD1;
    float4 BiNormal: TEXCOORD2;
    float4 PrevPosH:  TEXCOORD3;
    float4 CurrentPosH:  TEXCOORD4;
    float  Depth : TEXCOORD5;
    int ObjectId : TEXCOORD6;
};

// GBuffer pixel shader output
struct PS_Output_GBuffer
{
    float4 Depth : SV_Target0;
    float4 Compact: SV_Target1;
    float4 Diffuse: SV_Target2;
    float4 Specular: SV_Target3;
    float4 Motion: SV_Target4;
};

// simple pixel shader output
struct PS_Output_Simple
{
    float4 Color : SV_Target0;
};

#endif