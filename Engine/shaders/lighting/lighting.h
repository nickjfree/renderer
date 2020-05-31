// common effect data
#ifndef  __LIGHTING__
#define  __LIGHTING__

#include "../common/deferred.h"
#include "../common/basic_layout.h"
#include "../common/basic_registers.h"

/*
    shadow map
*/
Texture2D gShadowMap: register(t16);


/*
     image based lighting textures
*/

// image itself
TextureCube  gLightProbe           : register(t17);
// the look up texture
Texture2D    gLUT                  : register(t18);
// specular component. generated
TextureCube  gLdCube               : register(t19);
// diffuse component. the irradiance cude map generated
TextureCube  gLightProbeIrradiance : register(t20);

/*
     constant values
*/

#define  PI  3.141592657
#define  IBL_LD_MIPMAPS 10
#define  F90 1.0f


/*
    pbr helper functions
*/

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

    return num / (denom + 1e-5f);
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


float SelectLDMipmap(float Roughness)
{
    return lerp(0, IBL_LD_MIPMAPS - 1, Roughness);
}

float3 EnvBRDF(float3 SpecularColor, float Roughness, float NoV)
{
    // LUT is generated from IBLBaker
    float2 BRDF = gLUT.Sample(gSam, float2(NoV, 1-Roughness)).xy;
    return SpecularColor * BRDF.x + BRDF.y;
}

float3 SpecularIBL(float3 SpecularColor, float Roughness, float3 N, float3 V)
{

    float NoV = saturate(dot(N, V));
    float3 R = reflect(-V, N);
    float mipmap = SelectLDMipmap(Roughness);
    float3 WorldReflect = mul(float4(R, 0), gInvertViewMaxtrix).xyz;
    float3 PrefilteredColor = gLdCube.SampleLevel(gSam, WorldReflect, mipmap).xyz;
    float3 envBRDF = EnvBRDF(SpecularColor, Roughness, NoV);

    return PrefilteredColor * envBRDF;
}


/*
    get deferred lighing color from gbuffer
*/
float4 deferred_lighting(GBuffer gbuffer)
{
    // get vectors
    float3 normal = gbuffer.Normal.xyz;
    float3 position = gbuffer.Position;
    // get L, V, vectors
    float3 L = gLightPosition.xyz - position.xyz;
    L = normalize(L);
    float3 V = normalize(gbuffer.View);
    // calculate brdf   
    return BRDF(normal, V, L, gbuffer.Specular, F90, 
        gbuffer.Roughness, gbuffer.Diffuse.xyz, gbuffer.Metallic);
}

/*
    pixel in shadow or not?
*/

float shadow_value(GBuffer gbuffer)
{
    // shadow bias
    float bias = 0.0001f;
    // pixel position in view space
    float3 position = gbuffer.Position;
    // get position in light view space
    float4 light_position = mul(float4(position, 1), gInvertViewMaxtrix);
    light_position = mul(light_position, gLightViewProjection);
    float2 light_screen;
    light_screen.x = light_position.x / light_position.w / 2.0f + 0.5f;
    light_screen.y = -light_position.y / light_position.w / 2.0f + 0.5f;
    if ((saturate(light_screen.x) == light_screen.x) && (saturate(light_screen.y) == light_screen.y))
    {
        // check if cast shadow
        float depth = gShadowMap.Sample(gSam, light_screen).x;
        float depth_light = light_position.z / light_position.w - bias;
        if (depth_light < depth) {
            // in light
            return 1.0f;
        }
    }
    // in shadow
    return 0.0f;
}



#endif