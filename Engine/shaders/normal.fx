#ifndef __VSNORMAL__
#define __VSNORMAL__

#include "common.fx"

//the most common vertext shader
//using vertex type of vertex_dynamic_instancing
//BTW: every shaders use the same vertex format
struct VS_Input
{
	float3 PosL  : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    float  InstanceID : TEXCOORD1;
    float3 Tangent  : TANGENT;
};

struct VS_Input_Instance
{
    float3 PosL  : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    float  InstanceID : TEXCOORD1;
    float4 Tangent  : TANGENT;
    float4x4 InstanceWVP: InstanceWVP; 
    float4x4 InstanceWV: InstanceWV;
};

struct VS_Input_Skinning
{
    float3 PosL  : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    int  Bones : TEXCOORD1;
    float3 Weight : TEXCOORD2;
    float3 Tangent  : TANGENT;
};

struct PS_Input
{
	float4 PosH : SV_POSITION;
	float3 Normal : NORMAL;
	float2 TexCoord :TEXCOORD0;
};

struct PS_Output
{
	float4 Color : SV_Target0;
};

PS_Input VS(VS_Input input)
{
	PS_Input output = (PS_Input)0;
    // Transform to homogeneous clip space.
    output.PosH = mul(float4(input.PosL, 1.0f), gWorldViewProjection);
	output.TexCoord = input.TexCoord;// *float2(1, -1);
	output.Normal = input.Normal;
	return output;
}


PS_Input VS_Skinning(VS_Input_Skinning input)
{
    PS_Input output = (PS_Input)0;
    // apply matrix
    uint4 bones = uint4(
        input.Bones & 0x000000ff,
        input.Bones >> 8 & 0x000000ff,  
        input.Bones >> 16 & 0x000000ff,        
        input.Bones >> 24      
    );
    float4 weights = float4(input.Weight.xyz, 0);
    weights.w = 1.0f - (weights.x + weights.y + weights.z);


    // do skinning
    float4x4 mat0 = gSkinMatrix[bones.x];
    float4x4 mat1 = gSkinMatrix[bones.y];
    float4x4 mat2 = gSkinMatrix[bones.z];
    float4x4 mat3 = gSkinMatrix[bones.w];

    float4 position = float4(input.PosL, 1.0f);

    weights = float4(weights.x, weights.y, weights.z, weights.w);

    position = mul(position, mat0) * weights.x + mul(position, mat1) * weights.y 
        + mul(position, mat2) * weights.z + mul(position, mat3) * weights.w;

    output.PosH = mul(position, gWorldViewProjection);
    output.Normal = input.Normal;
    output.TexCoord = input.TexCoord;
    return output;
}


PS_Input VS_Instance(VS_Input_Instance input)
{
    PS_Input output = (PS_Input)0;
    // Transform to homogeneous clip space.
    output.PosH = mul(float4(input.PosL, 1.0f), input.InstanceWVP);
    output.TexCoord = input.TexCoord;
    output.Normal = input.Normal;
    return output;
}

PS_Output PS(PS_Input input)
{	
	PS_Output output = (PS_Output)0;
	//oColor = (light + 0.1);
	float4 light = gLightBuffer.Load(float3(input.PosH.xy, 0));
	float4 diffuse = gDiffuseMap0.Sample(gSam, input.TexCoord);
	//	float4 diffuse = gNormalBuffer.Sample(gSam, input.TexCoord);
	float3 color = diffuse.xyz *(light.xyz) + float3(0.1,0.1,0.1);
	float specular = light.w;
	output.Color = float4(color + light.xyz * specular, 0);
	return output;
}

#endif