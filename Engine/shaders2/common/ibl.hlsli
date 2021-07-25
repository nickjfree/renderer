#ifndef __IBL__
#define __IBL__

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

#endif