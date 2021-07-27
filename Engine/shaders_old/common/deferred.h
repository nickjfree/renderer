// common effect data
#ifndef  __DEFERRED__
#define  __DEFERRED__

// #include "common.h"
#include "basic_sampler.hlsli"
#include "basic_constant.h"
#include "post_constant.h"
#include "deferred_register.hlsli"


static const float fov = tan(0.15 * 3.141592654);

// deferred shading gbuffer

// gbuffer
struct GBuffer
{
    float3 Position;
    float4 Normal;
    float4 Diffuse;
    float3 View;
    float3 Specular;
    float Roughness;
    float Metallic;
    float ObjectId;
};


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
    float Depth  = gDepthBuffer.Sample(gSamPoint, uv).x;
    float3 Position = (GetLookVector(uv) * Depth).xyz;
    return Position;
}


// get view space position at uv in screen space
float3 GetPositionLoad(float2 uv)
{
    float Depth = gDepthBuffer.SampleLevel(gSamPoint, uv, 0).x;
    float3 Position = (GetLookVector(uv) * Depth).xyz;
    return Position;
}


// get the view space normal vector at uv in screen space
float4 GetNormal(float2 uv)
{
    float4 raw =  gCompactBuffer.Sample(gSamPoint, uv);
    return float4(DecodeNormal(raw.xy), 0);
}

// get the  space normal vector at uv in screen space
float4 GetNormalLoad(float2 uv)
{
    float4 raw =  gCompactBuffer.SampleLevel(gSamPoint, uv, 0);
    return float4(DecodeNormal(raw.xy), 0);
}

// sample gbuffer
GBuffer GetGBuffer(float2 uv)
{
    GBuffer gbuffer;

    // get vectors
    gbuffer.Position = GetPosition(uv);
    gbuffer.Normal = GetNormal(uv);
    gbuffer.Diffuse = gDiffuseBuffer.Sample(gSam, uv);
    gbuffer.View = normalize(-gbuffer.Position);
    // get roughness, specular and metallic value
    float4 rm = gSpecularBuffer.Sample(gSam, uv);
    gbuffer.Roughness = rm.y;
    gbuffer.Metallic = rm.z;
    float3 F0 = float3(rm.x, rm.x, rm.x);
    gbuffer.Specular = lerp(F0, gbuffer.Diffuse.rgb, gbuffer.Metallic);
    return gbuffer;
}

// sample gbuffer
GBuffer GetGBufferLoad(float2 uv)
{
    GBuffer gbuffer;

    // get vectors
    gbuffer.Position = GetPositionLoad(uv);
    gbuffer.Normal = GetNormalLoad(uv);
    gbuffer.Diffuse = gDiffuseBuffer.SampleLevel(gSamPoint, uv, 0);
    gbuffer.View = normalize(-gbuffer.Position);
    // get roughness, specular and metallic value
    float4 rm = gSpecularBuffer.SampleLevel(gSamPoint, uv, 0);
    gbuffer.Roughness = rm.y;
    gbuffer.Metallic = rm.z;
    float3 F0 = float3(rm.x, rm.x, rm.x);
    gbuffer.Specular = lerp(F0, gbuffer.Diffuse.rgb, gbuffer.Metallic);
    return gbuffer;
}

float2 GetPrevScreenCoord(float2 uv, out float valid)
{
    float4 motion = gMotionVector.Sample(gSamPoint, uv);
    valid = motion.w;
    return uv + motion.xy;
}

float2 GetPrevScreenCoordLoad(float2 uv, out float valid, out float fwidthZ)
{
    float4 motion = gMotionVector.SampleLevel(gSamPoint, uv, 0);
    valid = motion.w;
    fwidthZ = motion.z;
    return uv + motion.xy;
}


float GetObjectId(float2 uv) 
{
    return gCompactBuffer.Sample(gSamPoint, uv).w;
}

float GetLinearZLoad(float2 uv)
{
    return gCompactBuffer.SampleLevel(gSamPoint, uv, 0).z;
}

float reprojectionValid(float2 prevScreen, int objectId, float3 currentNormal, float currentLinearZ)
{
    float valid = 1.0;
    float4 compactData = gPrevCompactBuffer.Sample(gSam, prevScreen);
    if ((saturate(prevScreen.x) == prevScreen.x) && (saturate(prevScreen.y) == prevScreen.y)) {
        // check for object id
        if (abs(objectId - compactData.w) > 0.001f) return 0.0;
        // check for normal
        float3 prevNormal = DecodeNormal(compactData.xy);
        if (dot(prevNormal, currentNormal) < sqrt(2)/2.0) return 0.0;
        float prevLinearZ = compactData.z;
        // check for linear depth
        float maxChangeZ = max(abs(ddx(currentLinearZ)), abs(ddy(currentLinearZ)));
        if(abs(prevLinearZ - currentLinearZ) / (maxChangeZ + 1e-4) > 2.0) return 0.0;
    } else {
        return 0.0;   
    }
    return valid;
}

#endif