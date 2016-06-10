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

struct VS_InputTerrain
{
	float3 PosL  : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
	float  MorghFlag : TEXCOORD1;
    float4 Tangent  : TANGENT;
	float  Scale:     TEXCOORD2;
	float3 Center:    POSITION1;
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
	//input.Normal = float4(input.Normal.xyz, 0);
    output.Normal = mul(float4(input.Normal, 0),gWorldViewMatrix);
	//output.Normal = float4(input.Normal.xyz, 0);
	output.Tangent = mul(float4(input.Tangent.xyz, 0),gWorldViewMatrix);
	output.BiNormal = float4(cross(output.Normal.xyz, output.Tangent.xyz),0);
	output.BiNormal = normalize(output.BiNormal);
	output.TexCoord = input.TexCoord;
    output.Depth = pos.z;
	return output;
}

PS_Input VS_LPP_Skinning(VS_Input input)
{
	PS_Input output = (PS_Input)0;
    // Transform to homogeneous clip space.
    float4 pos;
    pos = float4(input.PosL,1.0f);
	input.Normal = float4(input.Normal.xyz,0);
	input.Tangent = float4(input.Tangent.xyz,0);
	//transform matrix
	//transform
	float4x4 TransMatrix = gSkinMatrix[input.InstanceID/4];
	pos = mul(pos,TransMatrix);
     //WVP
    output.PosH = mul(pos, gWorldViewProjection);
	//iNormal = mul(iNormal,Matrix);
	input.Normal = mul(input.Normal,TransMatrix);
	input.Tangent = mul(input.Tangent,TransMatrix);
	output.Tangent = mul(input.Tangent,gWorldViewMatrix);
    output.Normal = mul(input.Normal,gWorldViewMatrix);
	output.BiNormal = float4(cross(output.Normal.xyz, output.Tangent.xyz),0);
    output.Depth = mul(pos,gWorldViewMatrix).z;
	// texcoord, do not forget this for normal mapping
	output.TexCoord = input.TexCoord;
	return output;
}


PS_Output PS_LPP(PS_Input input)
{	
	PS_Output output = (PS_Output)0;
	//oColor = (light + 0.1);
	float4 normal = gNormalMap0.Sample(gSam,input.TexCoord);
	if(!any(normal))
	{
		normal = input.Normal;
	}
	else
	{
		normal = normal * 2.0 - 1;
		normal = input.Normal + normal.x * input.Tangent + normal.y * input.BiNormal;
	}
	//oColor = (light + 0.1);
	output.Normal = normalize(normal);
	output.Normal = output.Normal * 0.5 + 0.5;
	output.Depth.x = input.Depth;
	return output;
}

PS_Output PS_LPP_Normal(PS_Input input)
{	
	PS_Output output = (PS_Output)0;
	float4 normal = gNormalMap0.Sample(gSam,input.TexCoord);
	float4 diffuse = gDiffuseMap0.Sample(gSam, input.TexCoord);
	normal = normal * 2.0 - 1; 
	//oColor = (light + 0.1);
	normal = input.Normal + normal.x * input.Tangent + normal.y * input.BiNormal;
	output.Normal = normalize(normal);
	output.Normal = output.Normal * 0.5 + 0.5;
	output.Depth.x = input.Depth;
	output.Diffuse = diffuse;
//	output.Diffuse = float4(0.5,0.5,0.5,0);
	output.Specular = float4(0,0,0,0);
	return output;
}

PS_Output PS_LPP_Terrain(PS_Input input)
{	
	PS_Output output = (PS_Output)0;
	float4 normal = gNormalMap0.Sample(gSam,input.TexCoord);
	normal = normal * 2.0 - 1; 
	//oColor = (light + 0.1);
	normal = input.Normal;// + normal.x * input.Tangent + normal.y * input.BiNormal;
	output.Normal = normalize(normal);
	output.Normal = output.Normal * 0.5 + 0.5;
	output.Depth.x = input.Depth;
	return output;
}

#endif