#ifndef __LIGHTING__
#define __LIGHTING__

#include "brdf.hlsli"
#include "gbuffer.hlsli"


// IoR in air
#define F90 1.0f


#define LIGHT_TYPE_SPHERE       0
#define LIGHT_TYPE_DIRECTIONAL  1


/*
	light culling buffer element
*/

#define MAX_LIGHT_COUNT_PER_CELL  16
#define CELL_SCALE  1
#define CELL_COUNT  16

struct LightIndics
{
    uint numLights;
    uint lightIndics[15];
};

/*
	light context. (view space or world space)
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
	// representative point position
	float3 RepresentativePosition;
	// representative point direction
	float3 RepresentativeDirection;
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
	// world space
	float3 WorldSpacePosition;
	float3 WorldSpaceDirection;
};

/*
	get light context from constants
*/
LightContext getLightContextFromConstant(GBufferContext gbuffer)
{
	LightContext lightContext = (LightContext)0;

	// default to point light
	lightContext.Type = LIGHT_TYPE_SPHERE;
	lightContext.Color = gLightColor.xyz;
	// default to point light
	lightContext.Direction = normalize(gLightPosition.xyz - gbuffer.ViewSpacePosition.xyz);

	lightContext.Radius = gRadiusIntensity.x;
	lightContext.Intensity = gRadiusIntensity.y;
	lightContext.Position = gLightPosition.xyz;

	return lightContext;
}


// get addr of culling buffer from view space position
uint getLightBufferIndex(GBufferContext gbuffer) 
{
	float3 position = gbuffer.WorldSpacePosition;
	float3 viewPoint = gViewPoint.xyz;
	position -= viewPoint;
    position /= CELL_SCALE;
    // sign
    float3 region = sign(position);
    position = abs(position);
    position = max(position, 2);
    position = min(log2(position) - 1, CELL_COUNT / 2 - 1);
    // get the center
    position = floor(position) + 0.5;
    position = position * region;
    // id
    float3 center = floor(position + CELL_COUNT / 2);
    // get index
    uint address = (center.x + CELL_COUNT * (center.y + (center.z * CELL_COUNT)));

    return  address;
}

/*
	get light indics at current pixel
*/
LightIndics GetLights(GBufferContext gbuffer, StructuredBuffer<LightIndics> culledLights)
{
	uint addr = getLightBufferIndex(gbuffer);
	return culledLights[addr];
}

/*
	get light data
*/
LightData GetLightData(uint lightIndex) 
{
	return gLights[lightIndex];
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
	// TODO: get representative point position in view space

	if (lightData.Type == LIGHT_TYPE_DIRECTIONAL) {
		// directional light
		lightContext.Direction = mul(lightData.Direction, gViewMatrix).xyz;
	} else if (lightData.Type == LIGHT_TYPE_SPHERE) {
		// sphere/point/capsule, direction to light center
		lightContext.Direction = normalize(lightContext.Position - gbuffer.ViewSpacePosition);
		// TODO: get direction to representative point
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

	if(lightContext.Type == LIGHT_TYPE_SPHERE) {
		// point light
		falloff = saturate(1 - dot(d, d) / dot(lightContext.Radius, lightContext.Radius));
	} else if (lightContext.Type == LIGHT_TYPE_DIRECTIONAL) {
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
float3 GetLighting(GBufferContext gbuffer, uint lightIndex, inout float falloff)
{

	LightContext lightContext = getLightContext(gbuffer, lightIndex);
	float4 brdf = deferredBrdf(gbuffer, lightContext);
	// apply falloff and color and intaesity
	falloff = getFallOff(gbuffer, lightContext);
	float NoL = saturate(dot(gbuffer.ViewSpaceNormal, lightContext.Direction));
	float3 color = lightContext.Color * brdf.xyz;
	color = color * lightContext.Intensity * falloff * NoL;
	return color.xyz;
	// falloff = 1;
	// return float3(1, 1, 1);
}

#endif
