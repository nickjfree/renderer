#ifndef __VSSKINNING__
#define __VSSKINNING__

#include "common.fx"

//the most common vertext shader
//using vertex type of vertex_dynamic_instancing
//BTW: every shaders use the same vertex format
struct VS_Input
{
	float3 PosL  : POSITION;
    float4 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    float  InstanceID : TEXCOORD1;
    float4 Tangent  : TANGENT;
};

struct PS_Input
{
	float4 PosH : SV_POSITION;
	float4 Normal : NORMAL;
	float2 TexCoord :TEXCOORD0;
};

struct PS_Output
{
	float4 Color : SV_Target0;
};

PS_Input VS(VS_Input input)
{
	PS_Input output = (PS_Input)0;
	// fetch bone transform from constant buffer
	float4 pos = float4(input.PosL,1.0f);
	float4x4 TransMatrix = gSkinMatrix[input.InstanceID/4];
	pos = mul(pos, TransMatrix);
    output.PosH = mul(pos,gWorldViewProjection);
    output.TexCoord = input.TexCoord;
	output.Normal = input.Normal;
	return output;
}

PS_Output PS(PS_Input input)
{	
	//oColor = (light + 0.1);
	PS_Output output = (PS_Output)0;
	//oColor = (light + 0.1);
	float4  light = gLightBuffer.Load(float3(input.PosH.xy,0));
	float3 diffuse = gDiffuseMap0.Sample(gSam, input.TexCoord) * (light.xyz);
//	float4 diffuse = gNormalMap0.Sample(gSam, input.TexCoord) * float4(light.xyz,0);
	float specular = light.w;
//	output.Color = float4(diffuse + light.xyz * specular,0);
	output.Color = float4(diffuse.xyz,0);
	return output;
}

#endif