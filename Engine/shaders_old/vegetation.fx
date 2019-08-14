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
	float  Wind:     TEXCOORD2;
	float4 Row1: POSITION1;
	float4 Row2: POSITION2;
	float4 Row3: POSITION3;
	float4 Row4: POSITION4;
};


struct PS_Input_LPP
{
	float4 PosH : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
	float4 Normal : NORMAL;
	float4 Tangent : TEXCOORD1;
	float4 BiNormal: TEXCOORD2;
	float  Depth : TEXCOORD3;
};

struct PS_Output_LPP
{
	float4 Depth : SV_Target0;
	float4 Normal: SV_Target1;
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



float3 AnimateGrass(float3 position)
{
	float Height = position.y;
	if (Height > 0)
	{
		float t = gAbsoluteTime/500.0f;
		position.xz += 0.3 * sin(t * 3.14 * 2) * Height;
	}
	return position;
}


PS_Input VS(VS_Input input)
{
	PS_Input output = (PS_Input)0;
    // instance
	float Height = input.PosL.y;
	input.PosL = AnimateGrass(input.PosL);
	float4x4 Trans = float4x4(input.Row1, input.Row2, input.Row3, input.Row4);
	float4 pos = mul(float4(input.PosL, 1), Trans);
    output.PosH = mul(pos, gWorldViewProjection);
	output.Normal = mul(input.Normal, Trans);
	output.TexCoord = input.TexCoord;
	return output;
}

PS_Output PS(PS_Input input)
{	
	PS_Output output = (PS_Output)0;
	//oColor = (light + 0.1)
	float4  light = gLightBuffer.Load(float3(input.PosH.xy,0));
	//float4  normal = gNormalBuffer.Load(float3(input.PosH.xy,0));
	float4 diffuse = gDiffuseMap0.Sample(gSam, input.TexCoord);
//	clip(diffuse.w - 0.9f);
//	float specular = light.w;
	float3 color = diffuse.xyz *(light.xyz);
	float specular = light.w;
	output.Color = float4(color /* + light.xyz * specular*/, diffuse.w);
	return output;
}


PS_Input_LPP VS_LPP(VS_Input input)
{
	PS_Input_LPP output = (PS_Input_LPP)0;
	// instance
	input.PosL = AnimateGrass(input.PosL);
	float4x4 Trans = float4x4(input.Row1, input.Row2, input.Row3, input.Row4);
	float4 pos = mul(float4(input.PosL, 1), Trans);
	output.TexCoord = input.TexCoord;
	// Transform to homogeneous clip space.
	output.PosH = mul(pos, gWorldViewProjection);
	float4 pos_view = mul(pos, gWorldViewMatrix);
	input.Normal.w = 0;
	input.Tangent.w = 0;
	output.Normal = mul(input.Normal, Trans);
	output.Normal = mul(output.Normal, gWorldViewMatrix);
	output.Tangent = mul(input.Tangent, Trans);
	output.Tangent = mul(output.Tangent, gWorldViewMatrix);
	output.BiNormal = float4(cross(output.Normal.xyz, output.Tangent.xyz), 0);
	output.TexCoord = input.TexCoord;
	output.Depth = pos_view.z;
	return output;
}

PS_Output_LPP PS_LPP(PS_Input_LPP input)
{
	PS_Output_LPP output = (PS_Output_LPP)0;
	//oColor = (light + 0.1)
	//	float4  light = gLightBuffer.Load(float3(input.PosH.xy,0));
	//float4  normal = gNormalBuffer.Load(float3(input.PosH.xy,0));
	float4 diffuse = gDiffuseMap0.Sample(gSam, input.TexCoord);
	clip(diffuse.w - 0.9f);
	//oColor = (light + 0.1);
	output.Normal = normalize(input.Normal);
	output.Normal = output.Normal * 0.5 + 0.5;
	output.Depth.x = input.Depth;
	return output;
}
