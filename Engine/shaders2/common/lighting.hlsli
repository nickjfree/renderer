#ifndef __LIGHTING__
#define __LIGHTING__

#include "brdf.hlsli"
#include "gbuffer.hlsli"


// IoR in air
#define F90 1.0f

/*
    light data in constant buffer (raytracing or tiled lighting)
*/
struct LightData
{
    // light color
    float4 color;
    // light direction
    float4 direction;
    // light position
    float4 position;
    // lighting effect radius
    float radius;
    // intensity
    float intensity;
    // spotlight inner angle
    float capsuleDistanceOrInnerAngle; 
    // spotlight outer angle
    float capsuleRadiusOrOuterAngle;
    // rectlight width
    float rectLightWidth;
    // rectlight height
    float rectLightHeight;
    // light type
    uint type;
    // pad
    uint padLightData;
};

/*
    light constant buffer. deferred shading pass
*/

cbuffer PerLight: register(b0)
{
    // shadow map constant
    float4x4  gLightViewProjection;
    float4x4  gLightProjection;
    // lighting
    float4 gLightPosition;
    float4 gLightColor;
    float4 gRadiusIntensity;
    float4 gLightDirection;
}


/*
    light context. (view space)
*/
struct LightContext
{
	// light type
    uint type;
    // light color
    float4 color;
    // light direction
    float4 direction;
    // light position
    float4 position;
    // lighting effect radius
    float radius;
    // intensity
    float intensity;
    // spotlight inner angle
    float capsuleDistanceOrInnerAngle; 
    // spotlight outer angle
    float capsuleRadiusOrOuterAngle;
    // rectlight width
    float rectLightWidth;
    // rectlight height
    float rectLightHeight;
};

/*
	get light context from constants
*/
LightContext getLightContextFromConstant()
{

}

/*
	get light context from light index 
*/
LightContext getLightContext(uint lightIndex)
{

}

/*
    get deferred lighing color from gbuffer
*/
float4 DeferredBrdf(GBufferContext gbuffer, float3 viewSpaceLightDirection)
{
    // get vectors
    float3 normal = gbuffer.Normal.xyz;
    float3 position = gbuffer.Position;
    // get L, V, vectors
    // float3 L = gLightPosition.xyz - position.xyz;
    float3 L = normalize(viewSpaceLightDirection);
    float3 V = normalize(gbuffer.View);
    // calculate brdf   
    return BRDF(normal, V, L, gbuffer.Specular, F90, 
        gbuffer.Roughness, gbuffer.Diffuse.xyz, gbuffer.Metallic);
}


/*
	light data
*/

/*
    get falloff
*/
float getFallOff(LightData lightData, float3 position) 
{

    float3 d = lightData.position.xyz - position.xyz;
    float falloff = 1.0f;

    if(lightData.type == 0) {
        // point light
        falloff = saturate(1 - dot(d, d) / dot(lightData.radius, lightData.radius));
    } else if (lightData.type == 1) {
        // direction light
        falloff = 1.0f;
    } else {
        falloff = 1.0f;
    }
    return falloff;
}

/*
    lighting
*/
float3 GetLighting(LightData lightData, GBuffer gbuffer, inout float falloff)
{
    // get light direction
    float4 lightPosView = mul(lightData.position, gViewMatrix);
    float4 positionView = float4(gbuffer.Position, 1);
    float4 positionWorld = mul(positionView, gInvertViewMaxtrix);
    float3 L = float3(0, 0, 0);
    if (lightData.type == 0) {
        L =  lightPosView.xyz - positionView.xyz;
    } else if (lightData.type == 1) {
        L = -mul(float4(lightData.direction.xyz, 0), gViewMatrix).xyz;
    }
    float4 brdf = deferred_lighting(gbuffer, L);
    // apply falloff and color and intaesity
    falloff = getfalloff(lightData, positionWorld.xyz);
    float NoL = saturate(dot(gbuffer.Normal.xyz, L));
    float4 color = lightData.intensity * lightData.color * brdf * falloff * NoL;
    return color.xyz;
}

#endif
