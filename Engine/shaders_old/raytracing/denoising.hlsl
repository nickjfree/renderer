#ifndef DENOISING_HLSL
#define DENOISING_HLSL


#include "../common/deferred.h"
#include "../common/basic_layout.h"


Texture2D gPrevColor : register(t0);
Texture2D gPrevMoment : register(t1);
Texture2D gCurrentColor : register(t2);


Texture2D gColor : register(t0);
Texture2D gMoment : register(t1);


static const float maxAlphaColor = 0.05;
static const float maxAlphaMoment = 0.2;
static const float epsilon = 1e-6;

static const float h[25] = {1.0/256.0, 1.0/64.0, 3.0/128.0, 1.0/64.0, 1.0/256.0,
                              1.0/64.0,  1.0/16.0, 3.0/32.0,  1.0/16.0, 1.0/64.0,
                              3.0/128.0, 3.0/32.0, 9.0/64.0,  3.0/32.0, 3.0/128.0,
                              1.0/64.0,  1.0/16.0, 3.0/32.0,  1.0/16.0, 1.0/64.0,
                              1.0/256.0, 1.0/64.0, 3.0/128.0, 1.0/64.0, 1.0/256.0};

static const float kernelWeights[3] = { 1.0, 2.0 / 3.0, 1.0 / 6.0 };

static const float gaussKernel[9] = {1.0/16.0, 1.0/8.0, 1.0/16.0, 1.0/8.0, 1.0/4.0, 1.0/8.0, 1.0/16.0, 1.0/8.0, 1.0/16.0};

static const int step = 1.0;

static const float phiDepth = 1.0;
static const float normPower = 128.0;
static const float phiColor = 10.0;


float luma(float3 c){
  return dot(c, float3(0.2126, 0.7152, 0.0722));
}

bool eps_equal(float a, float b){
  return round(a) == round(b);
}


// float GetVariance(float2 uv) 
// {
//     float4 moments = gMoment.SampleLevel(gSamPoint, uv, 0);
//     float history = moments.w;

//     float variance = moments.y - moments.x * moments.x;
//     if (history < 4) {
//         variance = 1.0;
//     }
//     return abs(variance);
// }


float GetVariance(float2 uv, float2 texelSize) 
{

    float gvl = 0.001;
    for (int y0 = -1; y0 <= 1; y0++) {
        for (int x0 = -1; x0 <= 1; x0++) {
            float variance = gColor.SampleLevel(gSamPoint, uv + float2(x0, y0) * texelSize, 0).w;
            gvl += gaussKernel[x0 + 3*y0 + 4] * variance;
        }
    }
    float4 moments = gMoment.SampleLevel(gSamPoint, uv, 0);
    float history = moments.w;
    if (history < 4) {
        gvl = 1.0;
    }
    return abs(gvl);
}



/*
    weight function
*/
float computeWeight(
    float zCenter, float zP, float phiDepth,
    float3 normalCenter, float3 normalP, float phiNormal, 
    float luminanceCenter, float luminanceP, float phiColor)
{
    float wNormal    = pow(saturate(dot(normalCenter, normalP)), phiNormal);
    float wZ         = abs(zCenter - zP) / (phiDepth + epsilon);
    float wLdirect   = abs(luminanceCenter - luminanceP) / phiColor;

    float wDirect   = exp(0.0 - max(wLdirect, 0.0) - max(wZ, 0.0)) * wNormal;

    return wDirect;
}


/*
    pixel shader (temporal accumulation)
*/
PS_Output_Acc PS_TemporalAccumulation(PS_Input_Simple ps_input)
{   
    PS_Output_Acc output = (PS_Output_Acc)0;

    float2 uv = ps_input.TexCoord;

    float4 currentColor = gCurrentColor.SampleLevel(gSamPoint, uv, 0);
    // clamp
    currentColor = min(currentColor, float4(100, 100, 100, 0));
    float valid;
    float fwidthZ;
    float2 prevUV = GetPrevScreenCoordLoad(uv, valid, fwidthZ);
    float4 prevColor = gPrevColor.Sample(gSamPoint, prevUV, 0);
    float4 prevMoment = gPrevMoment.Sample(gSamPoint, prevUV, 0);
    float history = prevMoment.w;
    // accumulate history
    history = min( 32.0f, valid ? history + 1.0f : 1.0f );
    // current luminance
    float newLuma = luma(currentColor.xyz);

    float4 currentMoment = float4(float2(newLuma, newLuma * newLuma), 0, history);

    const float alphaColor  = valid ? max(maxAlphaColor,  1.0 / history) : 1.0;
    const float alphaMoment = valid ? max(maxAlphaMoment, 1.0 / history) : 1.0;

    // float alphaColor = max(maxAlphaColor, saturate(1.0 / (history + epsilon)) );
    // float alphaMoment = max(maxAlphaMoment, saturate(1.0 / (history + epsilon)) );

    // output accumulated color and moments
    output.Moment = float4(lerp(prevMoment, currentMoment, alphaMoment).xy, fwidthZ, history);
    // calc variants
    float variance = max(0, output.Moment.y - output.Moment.x * output.Moment.x);
    output.Color = float4(lerp(prevColor, currentColor, alphaColor).xyz, variance);
    return output;
}


/*
    pixel shader (filter variant)
*/
PS_Output_Simple PS_FilterVariant(PS_Input_Simple ps_input)
{
    PS_Output_Simple output = (PS_Output_Simple)0;
    // uv
    float2 uv = ps_input.TexCoord;
    // get history length
    float4 moments = gMoment.SampleLevel(gSamPoint, uv, 0);
    float history = moments.w;
    float  centerLinearZ = GetLinearZLoad(uv);
    // get texel size
    float2 demension;
    gColor.GetDimensions(demension.x, demension.y);
    float2 texelSize = 1 / demension;
    // skip background
    if (centerLinearZ <= 0)
    {
        // not a valid depth => must be envmap => do not filter
        output.Color = gColor.SampleLevel(gSamPoint, uv, 0);
        return output;
    }    
    if (history < 4) {
        // not enough history, filter variance spatialy
        float4 sumColor = float4(0, 0, 0, 0);
        float  sumWeights = 0;
        float4 sumMoments = float4(0, 0, 0, 0);
        // center pixel
        float4 centerColor = gColor.SampleLevel(gSamPoint, uv, 0);
        float3 centerNormal = GetNormalLoad(uv).xyz;
        float  centerLuminance = luma(centerColor.xyz);
        float  fwidthZ = gMoment.SampleLevel(gSamPoint, uv, 0).z;
        // phi
        float phiColor_ = phiColor;
        float phiDepth_ = max(fwidthZ, 1e-8) * 3;
        // kernel size
        int radius = 3;
        for (int yy = -radius; yy <= radius; yy++)
        {
            for (int xx = -radius; xx <= radius; xx++)
            {
                float2 loc = uv + float2(step * xx * texelSize.x, step * yy * texelSize.y);
                // fetch pixel data
                float4 color = gColor.SampleLevel(gSamPoint, loc, 0);
                float3 normal = GetNormalLoad(loc).xyz;
                float  luminance = luma(color.xyz);
                float4 moments = gMoment.SampleLevel(gSamPoint, loc, 0);
                float  fwidthZ = moments.z;
                float  linearZ = GetLinearZLoad(loc);
                // weight
                float w = computeWeight(
                    centerLinearZ, linearZ, phiDepth_ * length(float2(xx, yy)),
                    centerNormal, normal, normPower,
                    centerLuminance, luminance, phiColor_);
                // sum
                sumWeights += w;
                sumColor += color * w;
                sumMoments.xy += moments.xy * float2(w.xx);
            }
        }
        // 
        sumWeights = max(sumWeights, 1e-6);
        sumColor /= sumWeights;
        sumMoments.xy /= float2(sumWeights.xx);

        float variance = sumMoments.y - sumMoments.x * sumMoments.x;
        // give the variance a boost for the first frames
        variance *= 4.0 / history;
        output.Color = float4(sumColor.xyz, variance);
    } else {
        output.Color = gColor.SampleLevel(gSamPoint, uv, 0);
    }
    return output;
}


/*
    pixel shader (wavelet filter)
*/
PS_Output_Simple PS_Filter(PS_Input_Simple ps_input)
{   
    PS_Output_Simple output = (PS_Output_Simple)0;

    float2 uv = ps_input.TexCoord;

    // get center pixel
    float4 color = gColor.SampleLevel(gSamPoint, uv, 0);
    float3 normal = GetNormalLoad(uv).xyz;
    float  luminance = luma(color.xyz);
    float  fwidthZ = gMoment.SampleLevel(gSamPoint, uv, 0).z;
    float  linearZ = GetLinearZLoad(uv);
    // get texel size
    float2 demension;
    gColor.GetDimensions(demension.x, demension.y);
    float2 texelSize = 1 / demension;
    // center variance
    float variance = GetVariance(uv, texelSize);

    // phi
    float stepSize = 1;
    float phiColor_ = phiColor * sqrt(max(0.0, epsilon + variance));
    float phiDepth_ = max(fwidthZ, 1e-8) * stepSize;
    
    // skip background
    if (linearZ <= 0)
    {
        // not a valid depth => must be envmap => do not filter
        output.Color = color;
        return output;
    }    

    float4 sumColor = color;
    float sumWeights = 1.0;
    for (int offsetx = -2; offsetx <= 2; offsetx++) {
        for (int offsety = -2; offsety <= 2; offsety++) {

            float2 loc = uv + float2(step * offsetx * texelSize.x, step * offsety * texelSize.y);

            float kernel = kernelWeights[abs(offsetx)] * kernelWeights[abs(offsety)];

            if (offsetx != 0 || offsety != 0) // skip center pixel, it is already accumulated
            {
                // get normal
                float3 qNormal = GetNormalLoad(loc).xyz;
                // get linearZ
                float qLinearZ = GetLinearZLoad(loc);
                // get color
                float4 qColor = gColor.SampleLevel(gSamPoint, loc, 0);
                float qluminance = luma(qColor.xyz);
                // get weight
                float w = computeWeight(
                    linearZ, qLinearZ, phiDepth_ * length(float2(offsetx, offsety)),
                    normal, qNormal, normPower,
                    luminance, qluminance, phiColor_);
                // w * kernel
                w = w * kernel;

                sumWeights += w;
                sumColor += float4(w.xxx, w*w) * qColor;
            }
        }
    }
    // output.Color = float4((c / weights).xyz,  v / (weights * weights));
    output.Color = float4(sumColor / float4(sumWeights.xxx, sumWeights*sumWeights));
    return output;
}

#endif