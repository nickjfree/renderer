#ifndef __FXCRYSTAL__
#define __FXCRYSTAL__

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
	float4 Position : POSITION;
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
    // Transform to homogeneous clip space.
    output.PosH = mul(float4(input.PosL, 1.0f), gWorldViewProjection);
    output.TexCoord = input.TexCoord;
	input.Normal.w = 0;
    output.Normal = mul(input.Normal,gWorldViewMatrix);
	output.Position = mul(float4(input.PosL, 1.0f), gWorldViewMatrix);
	return output;
}

PS_Output PS(PS_Input input)
{	
	PS_Output output = (PS_Output)0;
	//oColor = (light + 0.1);
	input.Normal.y = -input.Normal.y;
	float2 uv =  -input.Normal.xy * 300  + input.PosH.xy; 
	float4  color = gPostBuffer.Load(float3(uv,0));
	float4 H = float4(normalize(input.Position.xyz),0);
	float specular = 3 * pow(saturate(dot(input.Normal,-H)),20);
	output.Color = color * 0.8 + 0.1 * specular * float4(1,1,1,0) + 0.1 * float4(0,1,0,0);
	return output;
}

#endif