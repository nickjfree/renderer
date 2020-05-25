#include "lighting.h"


/*
    point light rendering
*/
PS_Output_Simple PS_Point_Light(PS_Input_Simple ps_input)
{
    PS_Output_Simple output = (PS_Output_Simple)0;
    // sample gbuffer
    GBuffer gbuffer = GetGBuffer(ps_input.TexCoord);
    // shadow value, it is light intense
    float shadow = shadow_value(gbuffer);
    // lighting color
    float3 lighting_color = deferred_lighting(gbuffer).xyz;
    // adjust lighting by distance and light intensity
    float radius = gRadiusIntensity.x;
    float intensity = gRadiusIntensity.y;
    // pixel postion
    float3 position = gbuffer.Position;
    // get vectors
    float3 normal = gbuffer.Normal.xyz;
    // get L, V, vectors
    float3 L = gLightPosition.xyz - position.xyz;
    L = normalize(L);
    // distance from light
    float d = distance(gLightPosition.xyz, position.xyz);
    // an value
    float3 an = gLightColor.xyz * intensity * saturate(1 - d / radius);
    // final value
    output.Color = float4(lighting_color * an * saturate(dot(normal, L)), 0) * shadow;
    return output;
}


/*
    direction lighing (the sun)
*/
PS_Output_Simple PS_Direction_Light(PS_Input_Simple ps_input)
{
    PS_Output_Simple output = (PS_Output_Simple)0;
    GBuffer gbuffer = GetGBuffer(ps_input.TexCoord);
    // get vectors
    float3 normal = gbuffer.Normal.xyz;
    float3 position = gbuffer.Position;
    // get light view space texcoord
    float intensity = gRadiusIntensity.y;
    float3 L = -gLightDirection.xyz;
    L = normalize(L);
    // deferred lighting
    float3 lighting_color = gLightColor.xyz * deferred_lighting(gbuffer).xyz;
    // direction light shadow
    float shadow = gDiffuseMap0.Sample(gSam, ps_input.TexCoord).x;
    // final color
    output.Color = float4(intensity * lighting_color * saturate(dot(normal, L)), 0);
    
    output.Color = output.Color * (1 - shadow);
    return output;
}

PS_Output_Simple PS_ImageBased_Light(PS_Input_Simple ps_input)
{
    PS_Output_Simple output = (PS_Output_Simple)0;

    // get gbuffer
    GBuffer gbuffer = GetGBuffer(ps_input.TexCoord);
    // get normal and position
    float3 N = gbuffer.Normal.xyz;
    float3 position = gbuffer.Position;
    float3 V = gbuffer.View;

    if (length(position) < 0.001) {
        // draw the light probe
        float4 look = GetLookVector(ps_input.TexCoord);
        look = mul(look, gInvertViewMaxtrix);
        output.Color = gLightProbe.Sample(gSam, look.xyz);
        return output;
    }
    
    float NoV = saturate(dot(N, V));

    // get params
    float metallic = gbuffer.Metallic;
    float roughness = gbuffer.Roughness;

    float4 albedo = gbuffer.Diffuse;
    float3 WorldNormal = mul(float4(N.xyz, 0), gInvertViewMaxtrix).xyz;
    float3 irradiance = gLightProbeIrradiance.Sample(gSam, WorldNormal).rgb;

    // IBL Specular
    float3 SpecularColor = gbuffer.Specular;
    float3 kS = FresnelSchlickRoughness(SpecularColor, roughness, NoV);
    float3 specular = SpecularIBL(SpecularColor, roughness, N, V);


    // IBL Diffuse
    float3 diffuse = irradiance * albedo.xyz;
    float3 kD = 1 - kS;
    kD = lerp(kD, 0, metallic);

    float intensity = gRadiusIntensity.y;
    float4 color;
    color.rgb = (kD * diffuse + specular) * intensity;
    color.a = 1;
    output.Color = color;
    return output;
}
   