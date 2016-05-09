#ifndef __FXTERRAIN_REFLECT__
#define __FXTERRAIN_REFLECT__

#include "common.fx"

//the most common vertext shader
//using vertex type of vertex_dynamic_instancing
//BTW: every shaders use the same vertex format
struct VS_Input
{
	float3 PosL  : POSITION;
	float4 Normal : NORMAL;
	float2 TexCoord : TEXCOORD;
	float  MorghFlag : TEXCOORD1;
	float4 Tangent  : TANGENT;
	float  Scale : TEXCOORD2;
	float3 Center:    POSITION1;
};

struct PS_Input
{
	float4 PosH : SV_POSITION;
	float4 Position: TEXCOORD0;
	float4 Normal : NORMAL;
	float2 TexCoord :TEXCOORD1;
	float4 Alpha : DIFFUSE0;
};

struct PS_Output
{
	float4 Color : SV_Target0;
};

PS_Input VS(VS_Input input)
{
	PS_Input output = (PS_Input)0;
	// instance
	float3 world_pos = input.PosL * input.Scale + input.Center;
	input.PosL = BlendTerrain(world_pos, input.PosL, input.Center.y);
	float4 pos = float4(input.PosL * input.Scale + input.Center, 1);
	float Height = gHeightMap.Load(float3(pos.xz / 2, 0));
	pos.y = Height + 2;
	output.PosH = mul(pos, gWorldViewProjection);
	output.Position = mul(pos, gWorldViewMatrix);
	output.Normal = mul(float4(0,1,0,0), gWorldViewMatrix);
	output.TexCoord = input.TexCoord;
	output.Alpha = float4(0, 0, 0, 0);
	return output;
}

PS_Output PS(PS_Input input)
{	
	PS_Output output = (PS_Output)0;
	float3 clipos = input.PosH.xyz/input.PosH.w;
	float2 uv = clipos.xy * float2(0.5f,-0.5f) + 0.5;
	//oColor = (light + 0.1);
	float3 V = -normalize(input.Position.xyz);
	input.Normal = normalize(input.Normal);
	float alpha = dot(input.Normal.xyz,V);
	float4 color = float4(0,0,0,0);
	float4  light = gLightBuffer.Load(float3(input.PosH.xy,0));
	float3 diffuse = gDiffuseMap0.Sample(gSam, input.TexCoord) * (light.xyz + 0.3);
	if(alpha < 0.5)
	{
		float3 R = -reflect(V,input.Normal.xyz);
		float count = 32;
		float step = 1;
		float3 pos = input.Position.xyz;
		for(float i = 0;i < count;i++)
		{
			pos = pos + R * step;
			float4 SceenPos = mul(float4(pos,1),gProjectionMatrix);
			clipos = SceenPos.xyz/SceenPos.w;
			uv = clipos.xy * 0.5 + 0.5;
			uv.y = 1-uv.y;
			float depth = gDepthBuffer.Sample(gSam, uv);
			if(abs(depth - pos.z) < 0.5)
			{
				color = gPostBuffer.Sample(gSam, uv);
				break;
			}
		}
	}
	output.Color = color * 0.5 + 0.5 * float4(diffuse.xyz,0);
//	output.Color = input.Normal;
	return output;
}

#endif