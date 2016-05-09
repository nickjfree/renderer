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
	float  Scale:     TEXCOORD2;
	float3 Center:    POSITION1;
};

struct PS_Input
{
	float4 PosH : SV_POSITION;
	float4 Normal : NORMAL;
	float2 TexCoord :TEXCOORD0;
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
	pos.y = Height;
    output.PosH = mul(pos, gWorldViewProjection);
	output.Normal = input.Normal;
	output.TexCoord = input.TexCoord;
	output.Alpha = float4(0,0,0,0);
	return output;
}

PS_Output PS(PS_Input input)
{	
	PS_Output output = (PS_Output)0;
	//oColor = (light + 0.1)
	float4  light = gLightBuffer.Load(float3(input.PosH.xy,0));
	//float4  normal = gNormalBuffer.Load(float3(input.PosH.xy,0));
	float3 diffuse = gDiffuseMap0.Sample(gSam, frac(input.TexCoord/0.5));
	float specular = light.w;
	output.Color = float4(diffuse * light.xyz,0);
//	output.Color = float4(diffuse,0);
//	output.Color = light;
	return output;
}