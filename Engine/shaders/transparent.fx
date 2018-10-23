#ifndef __VSNORMAL__
#define __VSNORMAL__

#include "common.fx"
#include "oit.fx"

//the most common vertext shader
//using vertex type of vertex_dynamic_instancing
//BTW: every shaders use the same vertex format

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


PS_Output PS(PS_Input input)
{	
	PS_Output output = (PS_Output)0;
	float4 diffuse = gDiffuseMap0.Sample(gSam, input.TexCoord);
	float3 color = diffuse.xyz;
	output.Color = float4(color, 0.5);
    WriteNewPixelToAOIT(float2(0.5f, 0.5f), 0.5f, output.Color );
	return output;
}

#endif