#ifndef __FXREFLECT__
#define __FXREFLECT__

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
	float4 ScreenPos :TEXCOORD1;
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
	output.ScreenPos = output.PosH;
	return output;
}

PS_Output PS(PS_Input input)
{	
	PS_Output output = (PS_Output)0;
	float3 clipos = input.ScreenPos.xyz/input.ScreenPos.w;
	float2 uv = clipos.xy * float2(0.5f,-0.5f) + 0.5;
	//oColor = (light + 0.1);
	float3 V = -normalize(input.Position.xyz);
	input.Normal = normalize(input.Normal);
	float3 R = -reflect(V,input.Normal.xyz);
	output.Color = gPostBuffer.Sample(gSam, uv);
	int count = 32;
	int step = 1;
	float3 pos = input.Position.xyz;
	float4 color = float4(0,0,0,0);
	for(int i = 0;i < count;i++)
	{
		pos = pos + R * step;
		float4 SceenPos = mul(float4(pos,1),gProjectionMatrix);
		clipos = SceenPos.xyz/SceenPos.w;
		uv = clipos.xy * 0.5 + 0.5;
		uv.y = -uv.y;
		float depth = gDepthBuffer.Sample(gSam, uv);
		color = gPostBuffer.Sample(gSam, uv);
	}
	output.Color = color;
	return output;
}

#endif