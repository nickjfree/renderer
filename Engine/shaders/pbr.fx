#ifndef __VSNORMAL__
#define __VSNORMAL__

#include "common.fx"

// Physics Based Shading

#define  PI  3.141592657
#define  IBL_LD_MIPMAPS 8
#define  F90 1


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
float3 F_Schlick(in float3 f0, in float f90, in float HdotV)
{
	return f0 + (f90 - f0) * pow(1.f - HdotV, 5.f);
}

float3 FresnelSchlickRoughness(float3 F0, float roughness, float cosTheta)
{
    return F0 + (max(float(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
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


float DistributionGGX(float3 N, float3 H, float roughness) 
{
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = saturate(dot(N, H));
    float NdotH2 = NdotH * NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) 
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness) 
{
    float NdotV = saturate(dot(N, V));
    float NdotL = saturate(dot(N, L));
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
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
// roughness = (10-7 Smoothness)^6


float4 BRDF(float3 N, float3 V, float3 L, float3 f0, float f90, float roughness, float3 albedo, float metallic) {

	float NdotV = abs(dot(N, V)) + 1e-5f;
	// avoid artifact 
	float3 H = normalize(V + L);

	float LdotH = saturate(dot(L, H));
	float NdotH = saturate(dot(N, H));
	float NdotL = saturate(dot(N, L));

	// Specular BRDF
    float  NDF = DistributionGGX(N, H, roughness);        
    float  G   = GeometrySmith(N, V, L, roughness);      
    float3 F   = F_Schlick(f0, f90, saturate(dot(H, V)));      

	float3 numerator = NDF * G * F;
    float denominator = 4.0 * NdotV * NdotH;
    float3 specular = numerator / max(denominator, 0.001);

	// Diffuse BRDF 
	float3 kS = F;
	float3 kD = 1 - kS;
	kD = lerp(kD, 0, metallic);
	float3 diffuse = kD * albedo / PI;

	return float4(diffuse + specular, 0);
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
	float Bias = 0.0001f;
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
			float4 albedo = gDiffuseBuffer.Sample(gSam, input.TexCoord);
			float4 rm = gSpecularBuffer.Sample(gSam, input.TexCoord);
			float3 F0 = float3(rm.x, rm.x, rm.x);
			float metallic = rm.z;
			float roughness = rm.y;
			float3 SpecularColor = lerp(F0, albedo.rgb, metallic);
			float3 color = BRDF(Normal, V, L, SpecularColor, F90, roughness, albedo.xyz, metallic);
			float3 an = gLightColor * intensity * saturate(1 - d / radius);
			output.Light = float4(color * an * saturate(dot(Normal, L)), 0);
			return output;
		}
	}
	output.Light = float4(0, 0, 0, 0);
	return output;
}

PS_Output PS_DirectionLight(PS_Input input)
{
	PS_Output output = (PS_Output)0;
	float Bias = 0.0001f;
	float4 Normal = GetNormal(input.TexCoord);
	float3 Position = GetPosition(input.TexCoord);
	//Normal = Normal * 2 - 1;
	// get light view space texcoord
	float intensity = gRadiusIntensity.y;
	float3 L = -gLightDirection;
	float3 V = -Position.xyz;
	L = normalize(L);
	V = normalize(V);
	float3 H = normalize(L + V);
	float4 albedo = gDiffuseBuffer.Sample(gSam, input.TexCoord);
	float4 rm = gSpecularBuffer.Sample(gSam, input.TexCoord);
	float3 F0 = float3(rm.x, rm.x, rm.x);
	float metallic = rm.z;
	float roughness = rm.y;
	float3 SpecularColor = lerp(F0, albedo.rgb, metallic);
	float3 color = BRDF(Normal, V, L, SpecularColor, F90, roughness, albedo.xyz, metallic);
	output.Light = float4(color * gLightColor * saturate(dot(Normal, L)), 0) * intensity;


	// output.Light = albedo;
	return output;
}


float SelectLDMipmap(float Roughness)
{
	return lerp(0, IBL_LD_MIPMAPS - 1, Roughness);
}

float3 EnvBRDF(float3 SpecularColor, float Roughness, float NoV)
{
	float2 BRDF = gLUT.Sample(gSam, float2(Roughness, NoV));
	return SpecularColor * BRDF.x + BRDF.y;
}

float3 SpecularIBL(float3 SpecularColor, float Roughness, float3 N, float3 V)
{

	float NoV = saturate(dot(N, V));
	float3 R = 2 * dot(V, N) * N - V;

	float mipmap = SelectLDMipmap(Roughness);
	float4 WorldReflect = mul(float4(R, 0), gInvertViewMaxtrix);
	float3 PrefilteredColor = gLdCube.SampleLevel(gSam, WorldReflect, mipmap).rgb;
	float3 envBRDF = EnvBRDF(SpecularColor, Roughness, NoV);

	return PrefilteredColor * envBRDF;

}


// image based lighting shader, thanks to https://github.com/tuccio/IBLGGX
PS_Output PS_ImageBasedLight(PS_Input input)
{
	PS_Output output = (PS_Output)0;
	float4 N = GetNormal(input.TexCoord);
	float3 Position = GetPosition(input.TexCoord);

	if (length(Position) < 0.001) {
		// draw the light probe
		float4 look = GetLookVector(input.TexCoord);
		look = mul(look, gInvertViewMaxtrix);
		output.Light = gLightProbe.Sample(gSam, look.xyz);
		return output;
	}

	float3 V = -Position.xyz;
	V = normalize(V);
	float NoV = saturate(dot(N, V));


	float4 color;

	// get params
	float4 rm = gSpecularBuffer.Sample(gSam, input.TexCoord);
	float3 F0 = float3(rm.x, rm.x, rm.x);
	float metallic = rm.z;
	float roughness = rm.y;
	float4 albedo = gDiffuseBuffer.Sample(gSam, input.TexCoord);
    float4 WorldNormal = mul(float4(N.xyz, 0), gInvertViewMaxtrix);
	float3 irradiance = gLightProbeIrradiance.Sample(gSamBilinear, WorldNormal).rgb;

	// IBL Specular
	float3 SpecularColor = lerp(F0, albedo.rgb, metallic);
	float3 kS = FresnelSchlickRoughness(SpecularColor, roughness, NoV);
    float3 specular = SpecularIBL(SpecularColor, roughness, N, V);


	// IBL Diffuse
	float3 diffuse = irradiance * albedo.xyz;
	float3 kD = 1 - kS;
	kD = lerp(kD, 0, metallic);

	float intensity = gRadiusIntensity.y;

	color.rgb = (kD * diffuse + specular) * intensity;
	//color.rgb = (kD * diffuse) * intensity;
	// color.rgb = albedo;
	color.a = 1;
	output.Light = color;
	return output;
}

#endif