#ifndef __LIGHTING__
#define __LIGHTING__

#include "brdf.hlsli"
#include "gbuffer.hlsli"


// IoR in air
#define F90 1.0f

/*
    light data in constant buffer (raytracing or tiled lighting, world space)
*/
struct LightData
{
    // light color
    float4 Color;
    // light direction
    float4 Direction;
    // light position
    float4 Position;
    // lighting effect radius
    float Radius;
    // intensity
    float Intensity;
    // spotlight inner angle
    float CapsuleDistanceOrInnerAngle; 
    // spotlight outer angle
    float CapsuleRadiusOrOuterAngle;
    // rectlight width
    float RectLightWidth;
    // rectlight height
    float RectLightHeight;
    // light type
    uint Type;
    // pad
    uint Pad;
};

/*
    light constant buffer. one light
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
	light constant buffer multi lights
*/
cbuffer ArraylightInfos: register(b0, space0)
{
    LightData gLights[256];
}



/*
    light context. (view space)
*/
struct LightContext
{
	// light type
    uint Type;
    // light color
    float3 Color;
    // light direction
    float3 Direction;
    // light position
    float3 Position;
    // lighting effect radius
    float Radius;
    // intensity
    float Intensity;
    // spotlight inner angle
    float CapsuleDistanceOrInnerAngle; 
    // spotlight outer angle
    float CapsuleRadiusOrOuterAngle;
    // rectlight width
    float RectLightWidth;
    // rectlight height
    float RectLightHeight;
};

/*
	get light context from constants
*/
LightContext getLightContextFromConstant(GBufferContext gbuffer)
{
	LightContext lightContext = (LightContext)0;

	// default to point light
	lightContext.Type = 0;
	lightContext.Color = gLightColor.xyz;
	// default to point light
	lightContext.Direction = normalize(gLightPosition.xyz - gbuffer.ViewSpacePosition.xyz);

	lightContext.Radius = gRadiusIntensity.x;
	lightContext.Intensity = gRadiusIntensity.y;
	lightContext.Position = gLightPosition.xyz;

	return lightContext;
}

/*
	get light context from light index 
*/
LightContext getLightContext(GBufferContext gbuffer, uint lightIndex)
{
	LightContext lightContext = (LightContext)0;

	// lightData is in world space
	LightData lightData = gLights[lightIndex];

	// get infos
	lightContext.Type = lightData.Type;
	lightContext.Color = lightData.Color.xyz;
	lightContext.Radius = lightData.Radius;
	lightContext.Intensity = lightData.Intensity;
	lightContext.CapsuleDistanceOrInnerAngle = lightData.CapsuleDistanceOrInnerAngle;
    lightContext.CapsuleRadiusOrOuterAngle = lightData.CapsuleRadiusOrOuterAngle;
    lightContext.RectLightWidth = lightData.RectLightWidth;
    lightContext.RectLightHeight = lightData.RectLightHeight;

    // transform position and direction to view space
 	lightContext.Position = mul(lightData.Position, gViewMatrix).xyz;
    if (lightData.Type == 1) {
    	// directional light
    	lightContext.Direction = mul(lightData.Direction, gViewMatrix).xyz;
    } else if (lightData.Type == 0) {
    	// sphere/point/capsule, direction to light center
    	lightContext.Direction = normalize(mul(lightData.Direction, gViewMatrix).xyz - gbuffer.ViewSpacePosition);
    	// TODO: get direction to closest point
    }
	return lightContext;
}

/*
    get deferred lighing color
*/
float4 deferredBrdf(GBufferContext gbuffer, LightContext lightContext)
{
    // get vectors
    float3 normal = gbuffer.ViewSpaceNormal;
    float3 position = gbuffer.ViewSpacePosition;
    // get L, V, vectors
    // float3 L = gLightPosition.xyz - position.xyz;
    float3 L = normalize(lightContext.Direction);
    float3 V = normalize(gbuffer.ViewSpaceLookVector);
    // calculate brdf   
    return BRDF(normal, V, L, gbuffer.Specular, F90, 
        gbuffer.Roughness, gbuffer.Diffuse.xyz, gbuffer.Metallic);
}

/*
    get falloff
*/
float getFallOff(GBufferContext gbuffer, LightContext lightContext) 
{

    float3 d = lightContext.Position - gbuffer.ViewSpacePosition;
    float falloff = 1.0f;

    if(lightContext.Type == 0) {
        // point light
        falloff = saturate(1 - dot(d, d) / dot(lightContext.Radius, lightContext.Radius));
    } else if (lightContext.Type == 1) {
        // direction light
        falloff = 1.0f;
    } else {
        falloff = 1.0f;
    }
    return falloff;
}

/*
    lighting with no shadows
*/
float3 GetLighting(GBufferContext gbuffer, LightContext lightContext, inout float falloff)
{
    float4 brdf = deferredBrdf(gbuffer, lightContext);
    // apply falloff and color and intaesity
    falloff = getFallOff(gbuffer, lightContext);
    float NoL = saturate(dot(gbuffer.ViewSpaceNormal, lightContext.Direction));
    float3 color = lightContext.Color * brdf.xyz;
    color = color * lightContext.Intensity * falloff * NoL;
    return color.xyz;
}

#endif
