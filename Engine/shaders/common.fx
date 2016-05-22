// common effect data
#ifndef  __COMMON__
#define  __COMMON__

SamplerState gSam : register(s0);
SamplerState gSamBilinear :register(s1);
SamplerState gSamBoneMatrix :register(s2);

//multi texture, 8 texture at most 
Texture2D gDiffuseMap0 : register(t0);
Texture2D gDiffuseMap1 : register(t1);
Texture2D gDiffuseMap2 : register(t2);
Texture2D gDiffuseMap3 : register(t3);
Texture2D gNormalMap0 : register(t4);
Texture2D gNormalMap1 : register(t5);
Texture2D gNormalMap2 : register(t6);
Texture2D gNormalMap3 : register(t7);
Texture2D gSpecularMap : register(t8);
//a light buffer for light pre-pass,it seems CryEngine 2 do it this way
Texture2D gDepthBuffer : register(t9);
Texture2D gNormalBuffer: register(t10);
Texture2D gLightBuffer : register(t11);
Texture2D gPostBuffer : register(t12); 
Texture2D gFinalBuffer: register(t13);

// HDR lighting buffers
Texture2D gScaledBuffer : register(t14);
Texture2D gLumBuffer :  register(t15);
Texture2D gCurrentLum:  register(t16);
Texture2D gBloomBuffer :  register(t17);

//Shadow Maps
Texture2D gShadowMap: register(t18);
Texture2D gHeightMap: register(t19);


cbuffer MatrixBuffer : register(b0)
{
	float4x4 gWorldViewProjection;
	float4x4 gWorldInvertProjection;
	float4x4 gWorldViewMatrix;
	float4x4 gProjectionMatrix;
	float4x4 gInvertViewMaxtrix;
	float4   gViewPoint;
}

cbuffer LightViewBuffer : register(b1)
{
	float4x4  gLightViewProjection;
	float4x4  gLightView;
	float4x4  gLightProjection;
}

cbuffer SkinningBuffer : register(b2)
{
	// animation use 128 constants buffer,bone transform,update for each object draw
	float4x4  gSkinMatrix[128];
}

cbuffer SampleBuffer: register(b3)
{
	float4   gSampleOffsets[4];
	float4   gSampleWeights[4];
}

cbuffer Time: register(b4)
{
	// time elaps since previous frame
	int  gTimeElapse;
	int  gAbsoluteTime;
	float2 pad;
}

cbuffer Light: register(b5)
{
	float4 gLightPosition;
	float4 gLightColor;
	float4 gRadiusIntensity;
};

cbuffer Options: register(b6)
{
	float4 gScreenSize;
};




//float3 gLightDirection;



// bonematrix instancing use a texture which contains transform matrix for each frams,no need to update each object,but better not too big
Texture2D gBoneMatrix;
// shadow map for 8 lights,now only first is used and not implemented
Texture2D gShadowMaps[8];
// shader resourceview for each shadown map texture
cbuffer ShadowViewMatrixBuffer
{
	float4x4  gShadowViews[8];
}


static const float fov = tan(0.15 * 3.141592654);

static const float ranges[] = {32, 64, 128, 256, 512, 1024, 2048, 4096, 0, 0, 0, 0, 0, 0, 0 };


float4 GetLookVector(float2 uv)
{
	float2 clip_uv = (uv  - 0.5) * float2(2,-2);
	float fary = fov;
	float farx = gScreenSize.x / gScreenSize.y * fary;
	float4 LookVec = float4(float2(farx,fary) * clip_uv, 1,0);
	return LookVec;
}

float3 GetPosition(float2 uv)
{
	float Depth  = gDepthBuffer.Sample(gSam, uv);
	float3 Position = GetLookVector(uv) * Depth;
	return Position;
}

float4 GetNormal(float2 uv)
{
	float4 Normal =  gNormalBuffer.Sample(gSam, uv);
	Normal = Normal * 2 - 1;
	return Normal;
}


float GetTerrainBlend(float distance, int Lod)
{
	float far = ranges[Lod];
	float near = 0;
	if (Lod){
		near = ranges[Lod - 1];
	}
	near += (far - near) * 0.5;
	float blend = saturate((distance - near)/(far - near));
	return blend;
}


float3 BlendTerrain(float3 world_pos, float3 pos, int lod)
{
	float d = distance(gViewPoint.xyz, world_pos);
	float blend = GetTerrainBlend(d, lod);
	if (pos.x % 2) {
		pos.x = lerp(pos.x, pos.x + 1, blend);
	}
	if (pos.z % 2) {
		pos.z = lerp(pos.z, pos.z + 1, blend);
	}
	return pos;
}

#endif