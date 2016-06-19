#ifndef __VSNORMAL__
#define __VSNORMAL__

#include "common.fx"

// Physics Based Shading for point light

#define  PI  3.141592657;


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


// pbr functions
float3 F_Schlick(in float3 f0, in float f90, in float u)
{
	return f0 + (f90 - f0) * pow(1.f - u, 5.f);
}


float V_SmithGGXCorrelated(float NdotL, float NdotV, float alphaG)
{
	// Original formulation of G_SmithGGX Correlated 9 
	// lambda_v = (-1 + sqrt(alphaG2 * (1 - NdotL2) / NdotL2 + 1)) * 0.5f;  
	// lambda_l = (-1 + sqrt(alphaG2 * (1 - NdotV2) / NdotV2 + 1)) * 0.5f; 
	// G_SmithGGXCorrelated = 1 / (1 + lambda_v + lambda_l);  
	// V_SmithGGXCorrelated = G_SmithGGXCorrelated / (4.0f * NdotL * NdotV);

	// This is the optimize version 
	float alphaG2 = alphaG * alphaG;
	// Caution: the "NdotL *" and "NdotV *" are explicitely inversed , this is not a mistake. 
	float Lambda_GGXV = NdotL * sqrt((-NdotV * alphaG2 + NdotV) * NdotV + alphaG2);
	float Lambda_GGXL = NdotV * sqrt((-NdotL * alphaG2 + NdotL) * NdotL + alphaG2);

	return 0.5f / (Lambda_GGXV + Lambda_GGXL);
}

float D_GGX(float NdotH, float m)
{
	// Divide by PI is apply later 
	float m2 = m * m;
	float f = (NdotH * m2 - NdotH) * NdotH + 1;
	return m2 / (f * f);

}

float Fr_DisneyDiffuse(float NdotV, float NdotL, float LdotH, float linearRoughness)
{
	float energyBias = lerp(0, 0.5, linearRoughness);
	float energyFactor = lerp(1.0, 1.0 / 1.51, linearRoughness);
	float fd90 = energyBias + 2.0 * LdotH*LdotH * linearRoughness;
	float3 f0 = float3(1.0f, 1.0f, 1.0f);
	float lightScatter = F_Schlick(f0, fd90, NdotL).r;
	float viewScatter = F_Schlick(f0, fd90, NdotV).r;

	return lightScatter * viewScatter * energyFactor;
}





//// This code is an example of call of previous functions 
//
//float NdotV = abs(dot(N, V)) + 1e-5f; 
//// avoid artifact 
//float3 H = normalize(V + L);
//float LdotH = saturate(dot(L, H));
//float NdotH = saturate(dot(N, H)); 
//float NdotL = saturate(dot(N, L)); 
//
//// Specular BRDF
//float3 F = F_Schlick(f0, f90, LdotH); 
//float Vis = V_SmithGGXCorrelated(NdotV , NdotL , roughness); 
//float D = D_GGX(NdotH , roughness); 
//float Fr = D * F * Vis / PI; 
//
//// Diffuse BRDF 
//float Fd = Fr_DisneyDiffuse(NdotV , NdotL , LdotH , linearRoughness) / PI


// f0 = 0.16 reflectance2
// roughness = (1−0.7 Smoothness)^6



float4 Calc_PointLight(float3 N, float3 V, float3 L, float3 f0, float f90, float roughness, float3 albedo) {

	float NdotV = abs(dot(N, V)) + 1e-5f;
	// avoid artifact 
	float3 H = normalize(V + L);
	float LdotH = saturate(dot(L, H));
	float NdotH = saturate(dot(N, H));
	float NdotL = saturate(dot(N, L));

	// Specular BRDF
	float3 F = F_Schlick(f0, f90, LdotH);
	float Vis = V_SmithGGXCorrelated(NdotV, NdotL, roughness);
	float D = D_GGX(NdotH, roughness);
	float3 Fr = D * F * Vis / PI;

	// Diffuse BRDF 
	float Fd = Fr_DisneyDiffuse(NdotV, NdotL, LdotH, roughness) / PI;
	return albedo * Fd + Fr;
}




PS_Input VS_LightQuad(VS_Input input)
{
	PS_Input output = (PS_Input)0;
	output.PosH.xy = 2 * input.PosL.xy - 1;
	output.PosH.zw = 1;
	output.TexCoord = input.TexCoord;
	return output;
}

PS_Output PS_PointLightShadow(PS_Input input)
{
	PS_Output output = (PS_Output)0;
	float Bias = 0.001f;
	float4 Normal = GetNormal(input.TexCoord);
	float3 Position = GetPosition(input.TexCoord);
	//Normal = Normal * 2 - 1;
	// get light view space texcoord
	float4 LightViewPosition = mul(float4(Position, 1), gInvertViewMaxtrix);
	LightViewPosition = mul(LightViewPosition, gLightViewProjection);
	float2 LightTex;
	LightTex.x = LightViewPosition.x / LightViewPosition.w / 2.0f + 0.5f;
	LightTex.y = -LightViewPosition.y / LightViewPosition.w / 2.0f + 0.5f;
	if ((saturate(LightTex.x) == LightTex.x) && (saturate(LightTex.y) == LightTex.y))
	{
		// check if cast shadow
		float Depth = gShadowMap.Sample(gSam, LightTex);
		float DepthLight = LightViewPosition.z / LightViewPosition.w - Bias;
		if (DepthLight < Depth)
		{
			float radius = gRadiusIntensity.x;
			float intensity = gRadiusIntensity.y;
			float3 L = gLightPosition - Position.xyz;
			float3 V = -Position.xyz;
			L = normalize(L);
			V = normalize(V);
			float3 H = normalize(L + V);
			float d = distance(gLightPosition, Position.xyz);
			//float3 diffuse = gLightColor * saturate(dot(L, Normal.xyz));
			diffuse = diffuse * intensity * saturate(1 - d / radius);
			//float specular = intensity * pow(saturate(dot(Normal, H)), 20);
			//output.Light = float4(diffuse, specular);
			float4 albedo = gDiffuseBuffer.Sample(gSam, input.TexCoord);
			float3 f0 = float3(0.03, 0.03, 0.03, 0);
			float f90 = 1;
			float roughness = pow(1 - 0.7*0.5, 6);
			float3 color = Calc_PointLight(Normal, V, L, f0, f90, roughness, albedo.xyz)
				float3 an = gLightColor * intensity * saturate(1 - d / radius);
				output.Light = color * float4(an, 0);
				return output;
		}
		//output.Light = float4(1,1,1,0);
		// return output;
	}
	output.Light = float4(0, 0, 0, 0);
	return output;
}

#endif