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
    float4 Tangent  : TANGENT;
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

struct PS_Input
{
	float4 PosH : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
	float4 Normal : NORMAL;
	float4 Tangent : TEXCOORD1;
	float4 BiNormal: TEXCOORD2;
	float  Depth :TEXCOORD3;
};


struct PS_Output
{
	float4 Depth : SV_Target0;
	float4 Normal: SV_Target1;
	float4 Diffuse: SV_Target2;
	float4 Specular: SV_Target3;
};

PS_Input VS_LPP_Normal(VS_Input input)
{
	PS_Input output = (PS_Input)0;
    // Transform to homogeneous clip space.
    output.PosH = mul(float4(input.PosL, 1.0f), gWorldViewProjection);
	float4 pos = mul(float4(input.PosL,1.0f),gWorldViewMatrix);
    output.Normal = mul(float4(input.Normal, 0), gWorldViewMatrix);
	output.Tangent = mul(float4(input.Tangent.xyz, 0),gWorldViewMatrix);
	output.BiNormal = float4(cross(output.Normal.xyz, output.Tangent.xyz),0);
	output.BiNormal = normalize(output.BiNormal);
	output.Normal = normalize(output.Normal);
	output.Tangent = normalize(output.Tangent);
	output.TexCoord = input.TexCoord;
    output.Depth = pos.z;
	return output;
}

PS_Input VS_LPP_Normal_Instance(VS_Input_Instance input)
{
	PS_Input output = (PS_Input)0;
    // Transform to homogeneous clip space.
    output.PosH = mul(float4(input.PosL, 1.0f), input.InstanceWVP);
	float4 pos = mul(float4(input.PosL,1.0f), input.InstanceWV);
    output.Normal = mul(float4(input.Normal, 0), input.InstanceWV);
	output.Tangent = mul(float4(input.Tangent.xyz, 0), input.InstanceWV);
	output.BiNormal = float4(cross(output.Normal.xyz, output.Tangent.xyz),0);
	output.BiNormal = normalize(output.BiNormal);
	output.Normal = normalize(output.Normal);
	output.Tangent = normalize(output.Tangent);
	output.TexCoord = input.TexCoord;
    output.Depth = pos.z;
	return output;
}

PS_Output PS_LPP_Normal(PS_Input input)
{	
	PS_Output output = (PS_Output)0;
	float4 normal = gNormalMap0.Sample(gSam,input.TexCoord);
	float4 diffuse = gDiffuseMap0.Sample(gSam, input.TexCoord);
	normal = normal * 2.0 - 1;
	normal = normalize(normal);
	//oColor = (light + 0.1);
	normal = input.Normal + normal.x * input.Tangent + normal.y * input.BiNormal;
	output.Normal.xy = EncodeNormal(normal);
	output.Depth.x = input.Depth;
	output.Diffuse = diffuse;
	output.Specular = float4(0,0,0,0);
	return output;
}

#endif