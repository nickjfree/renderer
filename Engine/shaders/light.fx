#ifndef __VSNORMAL__
#define __VSNORMAL__

#include "common.fx"

//the most common vertext shader
//using vertex type of vertex_dynamic_instancing
//BTW: every shaders use the same vertex format

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
	float4 Light : SV_Target0;
};

PS_Input VS_ScreenQuadLight(VS_Input input)
{
	PS_Input output = (PS_Input)0;
	output.PosH.xy = 2 * input.PosL.xy - 1;
	output.PosH.zw = 1;
	output.TexCoord = input.TexCoord;
	return output;
}


PS_Output PS_ScreenQuadLight(PS_Input input)
{
	PS_Output output = (PS_Output)0;
	float Depth  = gDepthBuffer.Load(float3(input.PosH.xy,0));
	float4 Normal =  gNormalBuffer.Load(float3(input.PosH.xy,0));
	Normal = Normal * 2 - 1;
	float3 Position = GetPosition(input.TexCoord);
	float radius = gRadiusIntensity.x;
	float intensity = gRadiusIntensity.y;
	float3 L = gLightPosition - Position.xyz;
	float3 V = -Position.xyz;
	L = normalize(L);
	V = normalize(V);
	float3 H = normalize( L + V );
	float d = distance(gLightPosition,Position.xyz);
	float3 diffuse = gLightColor * saturate(dot(L, Normal.xyz));
	diffuse = diffuse * intensity * saturate(1 - d/radius);
	float specular = intensity * pow(saturate(dot(Normal,H)),20);
	output.Light = float4(diffuse,specular);
//	output.Light = Normal;
	return output;
}

#endif