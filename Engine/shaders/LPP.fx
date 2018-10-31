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

struct VS_Input_Skinning
{
    float3 PosL  : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    uint  Bones : TEXCOORD1;
    float3 Weight : TEXCOORD2;
    float3 Tangent  : TANGENT;
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

PS_Input VS_LPP_Normal_Skinning(VS_Input_Skinning input)
{
    PS_Input output = (PS_Input)0;
    // apply matrix
    uint4 bones = uint4(
        input.Bones & 0x000000ff,
        input.Bones >> 8 & 0x000000ff,  
        input.Bones >> 16 & 0x000000ff,        
        input.Bones >> 24      
    );
    float4 weights = float4(input.Weight.xyz, 0);
    weights.w = 1.0f - (weights.x + weights.y + weights.z);


    // do skinning
    float4x4 mat0 = gSkinMatrix[bones.x];
    float4x4 mat1 = gSkinMatrix[bones.y];
    float4x4 mat2 = gSkinMatrix[bones.z];
    float4x4 mat3 = gSkinMatrix[bones.w];

	float4 position = float4(input.PosL, 1.0f);
    float4 normal = float4(input.Normal, 0.0f);
    float4 tangent = float4(input.Tangent.xyz, 0);

    weights = float4(weights.x, weights.y, weights.z, weights.w);
    position = mul(position, mat0) * weights.x + mul(position, mat1) * weights.y 
        + mul(position, mat2) * weights.z + mul(position, mat3) * weights.w;

    normal = mul(normal, mat0) * weights.x + mul(normal, mat1) * weights.y 
        + mul(normal, mat2) * weights.z + mul(normal, mat3) * weights.w;
    normal = normalize(normal);

    tangent = mul(tangent, mat0) * weights.x + mul(tangent, mat1) * weights.y 
        + mul(tangent, mat2) * weights.z + mul(tangent, mat3) * weights.w;
    tangent = normalize(tangent);

    output.PosH = mul(position, gWorldViewProjection);
    float4 pos = mul(position, gWorldViewMatrix);
    output.Normal = mul(normal, gWorldViewMatrix);
    output.Tangent = mul(tangent, gWorldViewMatrix);

    output.BiNormal = float4(cross(output.Normal.xyz, output.Tangent.xyz),0);
    output.BiNormal = normalize(output.BiNormal);
    output.Normal = normalize(output.Normal);
    //output.Tangent = normalize(output.Tangent);
    //output.Tangent = float4(input.Bones >> 24, input.Bones >> 16 & 0x000000ff, input.Bones >> 8 & 0x000000ff, input.Bones & 0x000000ff);
    output.TexCoord = input.TexCoord;
    output.Depth = pos.z;
    return output;
}

PS_Output PS_LPP_Normal(PS_Input input)
{	
	PS_Output output = (PS_Output)0;
	float4 normal = gNormalMap0.Sample(gSam,input.TexCoord);
	float4 diffuse = gDiffuseMap0.Sample(gSam, input.TexCoord);
	float4 specular = gSpecularMap0.Sample(gSam, input.TexCoord);
	normal = normal * 2.0 - 1;
	normal.z = sqrt(1 - normal.x * normal.x - normal.y * normal.y);
	normal.w = 0;
	normal = normalize(normal);
	//oColor = (light + 0.1);
	normal = input.Normal * normal.z + normal.x * input.Tangent + normal.y * input.BiNormal;
	normal = normalize(normal);
	output.Normal.xy = EncodeNormal(normal);
	output.Depth.x = input.Depth;
	output.Diffuse = diffuse;
	//output.Diffuse = float4(1,1,1,0);
	output.Specular = float4(gSpecular, 1 - specular.y, specular.z, 0);
	return output;
}

PS_Output PS_LPP_Normal_Skinning(PS_Input input)
{   
    PS_Output output = (PS_Output)0;
    float4 normal = gNormalMap0.Sample(gSam,input.TexCoord);
    float4 diffuse = gDiffuseMap0.Sample(gSam, input.TexCoord);
    float4 specular = float4(1, 0.2f, 1, 0);
    normal = normal * 2.0 - 1;
    normal.z = sqrt(1 - normal.x * normal.x - normal.y * normal.y);
    normal.w = 0;
    normal = normalize(normal);
    //oColor = (light + 0.1);
    normal = input.Normal * normal.z + normal.x * input.Tangent + normal.y * input.BiNormal;
    normal = normalize(normal);
    output.Normal.xy = EncodeNormal(normal);
    output.Depth.x = input.Depth;
    output.Diffuse = float4(1,1,1,1);
    output.Specular = float4(gSpecular, 1 - specular.y, 0.0f, 0);
    return output;
}

#endif