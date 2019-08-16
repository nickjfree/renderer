// common effect data
#ifndef  __DEFERRED__
#define  __DEFERRED__

#include "basic_registers.h"
#include "basic_constant.h"
#include "post_constant.h"


static const float fov = tan(0.15 * 3.141592654);

// get view space look vector
float4 GetLookVector(float2 uv)
{
    float2 clip_uv = (uv  - 0.5) * float2(2,-2);
    float fary = fov;
    float farx = gScreenSize.x / gScreenSize.y * fary;
    float4 LookVec = float4(float2(farx,fary) * clip_uv, 1,0);
    return LookVec;
}

// encode normal to texture data
float2 EncodeNormal(float3 N) {
    N = normalize(N);
    float scale = 1.7777;
    float2 enc = N.xy / (N.z + 1);
    enc /= scale;
    enc = enc*0.5 + 0.5;
    return enc;

}

// decode texture data to normal
float3 DecodeNormal(float2 G) {

    float scale = 1.7777;
    float3 nn = float3(G.xy, 0) * float3(2 * scale, 2 * scale, 0) + float3(-scale, -scale, 1);
    float g = 2.0 / dot(nn.xyz, nn.xyz);
    float3 n;
    n.xy = g*nn.xy;
    n.z = g - 1;
    return n;
}

// get view space position at uv in screen space
float3 GetPosition(float2 uv)
{
    float Depth  = gDepthBuffer.Sample(gSamPoint, uv);
    float3 Position = GetLookVector(uv) * Depth;
    return Position;
}

// get the view space normal vector at uv in screen space
float4 GetNormal(float2 uv)
{
    float4 raw =  gNormalBuffer.Sample(gSamPoint, uv);
    return float4(DecodeNormal(raw), 0);
}





#endif