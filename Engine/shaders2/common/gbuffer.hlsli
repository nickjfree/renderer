// gbuffer
#ifndef  __GBUFFER__
#define  __GBUFFER__

/*
*	gbuffer textures
*/
Texture2D gDiffuseBuffer: register(t8);         // 4 bytes  xyz: color  w: flags
Texture2D gCompactBuffer: register(t9);         // 8 bytes  xy: encoded normal  z: linearZ(half)  w: objectId
Texture2D gDepthBuffer : register(t10);         // 4 bytes  linearZ(float)
Texture2D gSpecularBuffer: register(t11);       // 4 bytes  x: f0  y: roughness z: metalic w: reserved
Texture2D gMotionVector: register(t12);         // 8 bytes  xy: motion vector   z: fwidthZ w: reprojection valid
Texture2D gPrevCompactBuffer: register(t13);    // 8 bytes  xy: encoded normal  z: linearZ(half)  w: objectId

#include "constants.hlsli"

// gbuffer
struct GBufferContext
{
	// viewspace
    float3 ViewSpacePosition;
    float3 ViewSpaceNormal;
    float3 ViewSpaceLookVector;

    // worldspace
    float3 WorldSpacePosition;
    float3 WorldSpaceNormal;
    float3 WorldSpaceLookVector;
    // colors
    float4 Diffuse;
    float Specular;
    float Roughness;
    float Metallic;
    float ObjectId;
    // misc
    float ReprojecttionValid;
    float2 MotionVector;
    float FwidthZ;
    float LinearZ;
};

// get view space look vector (not normalized)
float3 getLookVector(float2 uv)
{
    float2 clipUV = (uv - 0.5) * float2(2, -2);
    float fary = FoV;
    float farx = gScreenSize.x / gScreenSize.y * fary;
    float3 look = float4(float2(farx, fary) * clipUV, 1);
    return look;
}


// encode normal to texture data
float2 encodeNormal(float3 N) {
    N = normalize(N);
    float scale = 1.7777;
    float2 enc = N.xy / (N.z + 1);
    enc /= scale;
    enc = enc*0.5 + 0.5;
    return enc;

}

// decode texture data to normal
float3 decodeNormal(float2 G) {

    float scale = 1.7777;
    float3 nn = float3(G.xy, 0) * float3(2 * scale, 2 * scale, 0) + float3(-scale, -scale, 1);
    float g = 2.0 / dot(nn.xyz, nn.xyz);
    float3 n;
    n.xy = g*nn.xy;
    n.z = g - 1;
    return n;
}

// get gbuffer data
GBufferContext GetGBufferContext(float uv) 
{
	GBufferContext gbuffer{};
	// linear depth  the z value
	float depth = gDepthBuffer.SampleLevel(gSamPoint, uv, 0).x;
	// view position
	gbuffer.ViewSpacePosition = getLookVector(uv) * depth;
	// view look
	gbuffer.ViewSpaceLookVector = normalize(-getLookVector(uv));
	// normal
	float4 compact =  gCompactBuffer.SampleLevel(gSamPoint, uv, 0);
    gbuffer.ViewSpaceNormal = decodeNormal(compact.xy);
    // get world space vectors 
    gbuffer.WorldSpacePosition = mul(float4(gbuffer.ViewSpacePosition, 1), gInvertViewMaxtrix).xyz;
    gbuffer.WorldSpaceNormal = mul(float4(gbuffer.ViewSpaceNormal, 0), gInvertViewMaxtrix).xyz
    gbuffer.WorldSpaceLookVector = mul(float4(gbuffer.ViewSpaceLookVector, 0), gInvertViewMaxtrix);
	// get diffuse
	gbuffer.Diffuse = gDiffuseBuffer.SampleLevel(gSamPoint, uv, 0);
	// get specular
	float4 rm = gSpecularBuffer.SampleLevel(gSamPoint, uv, 0);
    gbuffer.Roughness = rm.y;
    gbuffer.Metallic = rm.z;
    float3 F0 = float3(rm.x, rm.x, rm.x);
    gbuffer.Specular = lerp(F0, gbuffer.Diffuse.rgb, gbuffer.Metallic);
   	// object id
   	gbuffer.ObjectId = compact.w;
   	// linearz half
   	gbuffer.LinearZ = compact.z
   	// motions
   	float4 motion = gMotionVector.SampleLevel(gSamPoint, uv, 0);
   	gbuffer.ReprojecttionValid = motion.w;
   	gbuffer.MotionVector = motion.xy;
   	gbuffer.FwidthZ = motion.z;
}


// check  reprojection valid or not
float reprojectionValid(float2 prevScreen, int objectId, float3 currentNormal, float currentLinearZ)
{
    float valid = 1.0;
    float4 compactData = gPrevCompactBuffer.Sample(gSam, prevScreen);
    if ((saturate(prevScreen.x) == prevScreen.x) && (saturate(prevScreen.y) == prevScreen.y)) {
        // check for object id
        if (abs(objectId - compactData.w) > 0.001f) return 0.0;
        // check for normal
        float3 prevNormal = decodeNormal(compactData.xy);
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