#ifndef __SSAO__
#define __SSAO__



#include "../../common/gbuffer.hlsli"


// implement a ssao effect
//parameters
static const float random_size = 64;
static const float2 vec[4] = {float2(1,0),float2(-1,0),float2(0,1),float2(0,-1)};
static const float g_scale = 0.01f;
static const float g_bias = 0.2;
static const float g_intensity = 2;


Texture2D gDiffuseMap0 : register(t0); 

// post processing buffer
Texture2D gPostBuffer : register(t14); 


// sample random noise texture
float2 GetRandom(float2 uv)
{
    float4 Noise = gDiffuseMap0.Sample(gSam, frac(uv * gScreenSize.xy / random_size));
    Noise = Noise * 2 - 1;
    Noise = normalize(Noise);
    return Noise.xy;
}

float3 GetPosition(float2 uv)
{
    GBufferContext gbuffer = GetGBufferContext(uv);
    return gbuffer.ViewSpacePosition;
}


// calc ao value
float CalcAO(float2 coord, float3 p0, float3 n)
{
    float3 p1 = GetPosition(coord);
    float3 distance = p1 - p0;
    float3 v = normalize(distance);
    float d = length(distance) * g_scale;
    return max(0, dot(n,v) - g_bias) * (1.0f / (1.0f + d)) * g_intensity;
//  return p1;
}

/*
    ssao pixel shader
*/
float4 ShadePixel(GBufferContext gbuffer)
{   
    float2 uv = gbuffer.uv;

    float4 color = gPostBuffer.Sample(gSam, uv);
    float3 normal = gbuffer.ViewSpaceNormal;
    float3 position = gbuffer.ViewSpacePosition;

    if (length(position) < 0.001) {
        color.x = 1;
        return color;
    }

    float2 Noise = GetRandom(uv);
    float ao = 0;
    float radius = 0.1f;
    radius = radius / position.z;
    int iterations = 4;

    [unroll(4)]
    for(int i = 0; i < iterations; i++)
    {
        float2 coord1 = reflect(vec[i], Noise) * radius;
        //float2 coord1 = vec[i] *  radius;
        float2 coord2 = float2(coord1.x*0.707 - coord1.y*0.707,coord1.x*0.707 + coord1.y*0.707);
        ao += CalcAO(uv + coord1 * 0.25, position, normal);
        ao += CalcAO(uv + coord2 * 0.5, position, normal);
        ao += CalcAO(uv + coord1 * 0.75, position, normal);
        ao += CalcAO(uv + coord2, position, normal);
    }
    ao /= float(iterations) * 4;
    ao = saturate(ao);
    color.x = 1 - ao;

    if(isnan(color.x)) 
    {
        color.x = 1;
    }
    return color;
}

#endif
