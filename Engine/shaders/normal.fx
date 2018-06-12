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


PS_Input VS_Skinning(VS_Input input)
{
    PS_Input output = (PS_Input)0;
    // Transform to homogeneous clip space.
    output.PosH = mul(float4(input.PosL, 1.0f), gWorldViewProjection);
    output.TexCoord = input.TexCoord;// *float2(1, -1);
    output.Normal = input.Normal;
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