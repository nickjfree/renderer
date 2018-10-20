// common effect data
#ifndef  __COMMON__
#define  __COMMON__

SamplerState gSam : register(s0);
SamplerState gSamBilinear :register(s1);
SamplerState gSamPoint :register(s2);

//multi texture, 8 texture at most 
Texture2D gDiffuseMap0 : register(t0);
Texture2D gDiffuseMap1 : register(t1);
Texture2D gDiffuseMap2 : register(t2);
Texture2D gDiffuseMap3 : register(t3);
Texture2D gNormalMap0 : register(t4);
Texture2D gNormalMap1 : register(t5);
Texture2D gNormalMap2 : register(t6);
Texture2D gNormalMap3 : register(t7);
Texture2D gSpecularMap0 : register(t8);

// defferd shading GBuffer
Texture2D gDepthBuffer : register(t9);
Texture2D gNormalBuffer: register(t10);
Texture2D gDiffuseBuffer: register(t11);
Texture2D gSpecularBuffer: register(t12);
Texture2D gLightBuffer : register(t13);


// misc textures


// pingpong buffers
Texture2D gPostBuffer : register(t14); 
Texture2D gFinalBuffer: register(t15);

Texture2D gShadowMap: register(t16);
// image based light
TextureCube  gLightProbe		   : register(t17);
Texture2D	 gLUT                  : register(t18);
TextureCube  gLdCube               : register(t19);
TextureCube  gLightProbeIrradiance : register(t20);

RasterizerOrderedTexture2D<float> gTest : register(u0);


cbuffer PerObject: register(b0)
{
	// object
	float4x4 gWorldViewProjection;
	float4x4 gWorldInvertProjection;
	float4x4 gWorldViewMatrix;
    // material
	float gSpecular;
	float3 pad_1;
}

cbuffer PerLight: register(b1)
{
	// shadow map constant
	float4x4  gLightViewProjection;
	float4x4  gLightView;
	float4x4  gLightProjection;
	// lighting
	float4 gLightPosition;
	float4 gLightColor;
	float4 gRadiusIntensity;
	float4 gLightDirection;
}


cbuffer PerFrame: register(b2) 
{
	// per frame
	float4x4 gProjectionMatrix;
	float4x4 gInvertViewMaxtrix;
	float4   gViewPoint;

	int  gTimeElapse;
	int  gAbsoluteTime;
	float2 pad;
	// post pass 
	float4   gSampleOffsets[4];
	float4   gSampleWeights[4];
}

cbuffer ArrayAnime: register(b3)
{
	// animation use 128 constants buffer,bone transform,update for each object draw
	float4x4  gSkinMatrix[128];
}

cbuffer Misc : register(b4)
{
	float4 gScreenSize;
}

//float3 gLightDirection;


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

float2 EncodeNormal(float3 N) {
	N = normalize(N);
	//float2 enc;
	//enc = normalize(N.xy)*sqrt(abs(N.z*0.5 + 0.5));
	//return enc * 0.5 + 0.5;


	//return N.xy * 0.5 + 0.5;

	float scale = 1.7777;
	float2 enc = N.xy / (N.z + 1);
	enc /= scale;
	enc = enc*0.5 + 0.5;
	return enc;

}

float3 DecodeNormal(float2 G) {
	//float4 N = float4(G * float4(2, 2, 0, 0) + float4(-1, -1, 1, -1),0,0);
	//float l = dot(N.xyz, -N.xyw);
	//N.z = l;
	//N.xy *= sqrt(l);
	//return N.xyz * 2 + float3(0, 0, -1);
	
	/*float3 N;
	G = G * 2 - 1;
	N.z = dot(G.xy, G.xy) * 2 - 1;
	N.xy = normalize(G.xy)*sqrt(abs(1 - N.z*N.z));
	return float3(N.x, N.y, N.z);*/
	//G = G * 2 - 1;
	//float3 n = float3(G.xy, -sqrt(1- dot(G.xy, G.xy)));
	//return n;
//	return normalize(n);

	float scale = 1.7777;
	float3 nn = float3(G.xy, 0) * float3(2 * scale, 2 * scale, 0) + float3(-scale, -scale, 1);
	float g = 2.0 / dot(nn.xyz, nn.xyz);
	float3 n;
	n.xy = g*nn.xy;
	n.z = g - 1;
	return n;
}

float3 GetPosition(float2 uv)
{
	float Depth  = gDepthBuffer.Sample(gSamPoint, uv);
	float3 Position = GetLookVector(uv) * Depth;
	return Position;
}

float4 GetNormal(float2 uv)
{
	float4 raw =  gNormalBuffer.Sample(gSamPoint, uv);
	return float4(DecodeNormal(raw), 0);
	//Normal = Normal * 2 - 1;
	//Normal = float4(normalize(Normal.xyz), 0);
	//return Normal;
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