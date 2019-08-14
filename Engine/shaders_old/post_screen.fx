#ifndef __VSPOST__
#define __VSPOST__

#include "common.fx"

//the most common vertext shader
//using vertex type of vertex_dynamic_instancing
//BTW: every shaders use the same vertex format

// implement a ssao effect
//parameters
static const float random_size = 64;
static const float2 vec[4] = {float2(1,0),float2(-1,0),float2(0,1),float2(0,-1)};
static const float g_scale = 0.01f;
static const float g_bias = 0.2;
static const float g_intensity = 2;


struct VS_Input
{
	float3 PosL  : POSITION;
	float2 TexCoord : TEXCOORD;
};

struct PS_Input
{
	float4 PosH : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};

struct PS_Output
{
	float4 Color : SV_Target0;
};

PS_Input VS(VS_Input input)
{
	PS_Input output = (PS_Input)0;
	output.PosH.xy = 2 * input.PosL.xy - 1;
	output.PosH.zw = 1;
	output.TexCoord = input.TexCoord;
	return output;
}

float2 GetRandom(float2 uv)
{
	float4 Noise = gDiffuseMap0.Sample(gSam, frac(uv * gScreenSize / random_size));
	Noise = Noise * 2 - 1;
	Noise = normalize(Noise);
	return Noise.xy;
}

float CalcAO(float2 coord, float3 p0, float3 n)
{
	float3 p1 = GetPosition(coord);
	float3 distance = p1 - p0;
	float3 v = normalize(distance);
	float d = length(distance) * g_scale;
	return max(0, dot(n,v) - g_bias) * (1.0f / (1.0f + d)) * g_intensity;
//	return p1;
}

PS_Output PS(PS_Input input)
{	
	PS_Output output = (PS_Output)0;
	float2 uv = input.TexCoord;
	//oColor = (light + 0.1);
	float4 Color = gPostBuffer.Sample(gSam, input.TexCoord);
	float4 Normal = GetNormal(uv);
	float3 Position = GetPosition(uv);

	if (length(Position) < 0.001) {
		output.Color = Color;
		return output;
	}

	float2 Noise = GetRandom(uv);
	float ao = 0;
	float radius = 0.1f;
	radius = radius/Position.z;
	int iterations = 4;
	for(int i = 0; i < iterations; i++)
	{
		float2 coord1 = reflect(vec[i], Noise) * radius;
		//float2 coord1 = vec[i] *  radius;
		float2 coord2 = float2(coord1.x*0.707 - coord1.y*0.707,coord1.x*0.707 + coord1.y*0.707);
		ao += CalcAO(uv + coord1 * 0.25, Position, Normal);
		ao += CalcAO(uv + coord2 * 0.5, Position, Normal);
		ao += CalcAO(uv + coord1 * 0.75, Position, Normal);
		ao += CalcAO(uv + coord2, Position, Normal);
	}
	ao /= float(iterations) * 4;
	ao = saturate(ao);
	output.Color = lerp(Color, 0.1 * Color, ao);
//	if(Color.y == 0.5 && Color.x == 0 && Color.z == 0)
//	{
//		output.Color = Color;
//	}
	if(isnan(output.Color.x)) 
	{
		output.Color = Color;
	}
	//output.Color = Normal;
	// output.Color = Color;
	return output;
}

#endif