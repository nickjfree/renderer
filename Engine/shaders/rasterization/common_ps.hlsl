#ifndef __COMMON_PS__
#define __COMMON_PS__


#include "common.hlsli"


Texture2D gDiffuseMap0 : register(t0);
Texture2D gNormalMap0 : register(t4);
Texture2D gSpecularMap0 : register(t7);

/*
	uniform pixel shader
*/
PS_Output PSMain(PS_Input psInput)
{   
	PS_Output output = (PS_Output)0;

#ifdef VS_CLIPMAP
	float4 normal = float4(0.5, 0.5, 1, 0);
	float4 diffuse = float4(gTerrainVirtualTexture.Sample(gSamPoint, psInput.TexCoord).xyz, 1);
	float4 specular = float4(0, 0.3, 0, 0);
#else
	// sample textures
	float4 diffuse = gDiffuseMap0.Sample(gSam, psInput.TexCoord);
	float4 normal = gNormalMap0.Sample(gSam, psInput.TexCoord);
	float4 specular = gSpecularMap0.Sample(gSam, psInput.TexCoord);
#endif
	// normalize normal vector
	normal = normal * 2.0 - 1;
	normal.z = sqrt(1 - normal.x * normal.x - normal.y * normal.y);
	normal.w = 0;
	normal = normalize(normal);
	normal = psInput.Normal * normal.z + normal.x * psInput.Tangent + normal.y * psInput.BiNormal;
	normal = normalize(normal);

#ifdef RENDER_GBUFFER
	// output
	output.Compact.xy = encodeNormal(normal.xyz);
	// depth: view space z value
	output.Depth.x = psInput.Depth;
	output.Diffuse = diffuse;
	// x: specular  y: roughness  z: metallic
	output.Specular = float4(gSpecular, specular.y, specular.z, 0);
	// output.Specular = float4(gSpecular, 0.1, specular.z, 0);
	// compact info: object id
	output.Compact.w = psInput.ObjectId;

	// output.Specular = float4(gSpecular, specular.y, 1, 0);
	// motion vectors
	float2 currentScreen = psInput.CurrentPosH.xy / psInput.CurrentPosH.w * 0.5 + 0.5;
	float2 prevScreen = psInput.PrevPosH.xy / psInput.PrevPosH.w * 0.5 + 0.5;   
	float  linearZ = psInput.CurrentPosH.z;
	currentScreen.y = 1 - currentScreen.y;
	prevScreen.y = 1 - prevScreen.y;
	// motion vector valid or not
	float valid = reprojectionValid(prevScreen, psInput.ObjectId, normal.xyz, linearZ);
	// z grad
	float fwidthZ = max(abs(ddx(linearZ)), abs(ddy(linearZ)));
	//set motion vector and compact buffer,  xy: motion, z: fwidthZ, w: isValid
	output.Motion = float4(prevScreen.xy - currentScreen.xy, fwidthZ, valid);
	// history length
	output.Compact.z = linearZ;
#else
	output.Color = diffuse;
#endif
	return output;
}

#endif