#ifndef __RESOLVE__
#define __RESOLVE__



#include "../../common/gbuffer.hlsli"
#include "../../common/ibl.hlsli"

// raytracing result
Texture2D gRaytracedReflection : register(t0); 
Texture2D gAO : register(t1); 
Texture2D gRaytracedLighting : register(t2); 

// post processing buffer
Texture2D gPostBuffer : register(t3); 

/*
    reflection resolve pixel shader
*/
float4 ShadePixel(GBufferContext gbuffer)
{   

    float roughness = gbuffer.Roughness;
    float3 N = gbuffer.ViewSpaceNormal;
    float3 V = gbuffer.ViewSpaceLookVector;
    float3 specularColor = gbuffer.Specular;
    float NoV = saturate(dot(N, V));

    float3 reflection = gRaytracedReflection.Sample(gSam, gbuffer.uv).xyz;
    // pre-intergrated texture
    reflection =  reflection * EnvBRDF(specularColor, roughness, NoV);
    // lighting
    float3 lighting = gPostBuffer.Sample(gSam, gbuffer.uv).xyz;
    // add raytraced lighting (test code)
    lighting += gRaytracedLighting.Sample(gSam, gbuffer.uv).xyz;
    float ao = gAO.Sample(gSam, gbuffer.uv).x;

    float4 color = float4(reflection + lighting * ao, 0);
    return color;
}


#endif
