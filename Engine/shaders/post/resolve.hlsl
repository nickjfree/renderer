
#include "../common/deferred.h"
#include "../common/basic_layout.h"
#include "../lighting/lighting.hlsli"
#include "../common/post.h"

// raytracing result
Texture2D gRaytracedReflection : register(t0); 
Texture2D gAO : register(t1); 
Texture2D gRaytracedLighting : register(t2); 

/*
    reflection resolve pixel shader
*/
PS_Output_Simple PS(PS_Input_Simple input)
{   
    PS_Output_Simple output = (PS_Output_Simple)0;
    float2 uv = input.TexCoord;

   // float4 Color = gPostBuffer.Sample(gSam, input.TexCoord);

    GBuffer gbuffer = GetGBuffer(uv);

    float roughness = gbuffer.Roughness;
    float3 N = gbuffer.Normal.xyz;
    float3 V = gbuffer.View;
    float3 specularColor = gbuffer.Specular;
    float NoV = saturate(dot(N, V));

    float3 reflection = gRaytracedReflection.Sample(gSam, input.TexCoord).xyz;
    // pre-intergrated texture
    reflection =  reflection * EnvBRDF(specularColor, roughness, NoV);
    // lighting
    float3 lighting = gPostBuffer.Sample(gSam, input.TexCoord).xyz;
    // add raytraced lighting (test code)
    lighting += gRaytracedLighting.Sample(gSam, input.TexCoord).xyz;
    float ao = gAO.Sample(gSam, input.TexCoord).x;

    output.Color = float4((reflection + lighting * ao), 0);
    return output;
}
