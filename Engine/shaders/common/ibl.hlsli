#ifndef __IBL__
#define __IBL__

/*
     image based lighting textures
*/

#include "lighting.hlsli"


// image itself
TextureCube  gLightProbe           : register(t17);
// the look up texture
Texture2D    gLUT                  : register(t18);
// specular component. generated
TextureCube  gLdCube               : register(t19);
// diffuse component. the irradiance cude map generated
TextureCube  gLightProbeIrradiance : register(t20);


#define IBL_LD_MIPMAPS  10


float SelectLDMipmap(float Roughness)
{
    return lerp(0, IBL_LD_MIPMAPS - 1, Roughness);
}

float3 EnvBRDF(float3 SpecularColor, float Roughness, float NoV)
{
    // LUT is generated from IBLBaker
    float2 BRDF = gLUT.SampleLevel(gSam, float2(NoV, 1-Roughness), 0).xy;
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

// IBL lighting. lighting of the env map
float4 ShadePixelIBL(GBufferContext gbuffer)
{

    LightContext light = getLightContextFromConstant(gbuffer);
    // get normal and position
    float3 N = gbuffer.ViewSpaceNormal;
    float3 V = gbuffer.ViewSpaceLookVector;
    float3 position = gbuffer.ViewSpacePosition;

    // ignore background pixels
    if (length(position) < 0.001) {
        // draw the env map
        float4 color = gLightProbe.Sample(gSam, -gbuffer.WorldSpaceLookVector) * light.Intensity;
        return color;
    }
    
    float NoV = saturate(dot(N, V));

    // get params
    float metallic = gbuffer.Metallic;
    float roughness = gbuffer.Roughness;
    float4 albedo = gbuffer.Diffuse;

    // irrandiance off current pixel
    float3 irradiance = gLightProbeIrradiance.Sample(gSam, gbuffer.WorldSpaceNormal).rgb;

    // IBL Specular
    float3 SpecularColor = gbuffer.Specular;
    float3 kS = FresnelSchlickRoughness(SpecularColor, roughness, NoV);
    float3 specular = SpecularIBL(SpecularColor, roughness, N, V);

    // ignore specular. replaced with raytracing
    specular = specular * 0.0f;

    // IBL Diffuse
    float3 diffuse = irradiance * albedo.xyz;
    float3 kD = 1 - kS;
    kD = lerp(kD, 0, metallic);

    float4 color;
    color.rgb = (kD * diffuse + specular) * light.Intensity;
    color.a = 1;

    return color;
}


#endif