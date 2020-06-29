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
static const float epsilon = 1e-5;

static const float h[25] = {1.0/256.0, 1.0/64.0, 3.0/128.0, 1.0/64.0, 1.0/256.0,
                              1.0/64.0,  1.0/16.0, 3.0/32.0,  1.0/16.0, 1.0/64.0,
                              3.0/128.0, 3.0/32.0, 9.0/64.0,  3.0/32.0, 3.0/128.0,
                              1.0/64.0,  1.0/16.0, 3.0/32.0,  1.0/16.0, 1.0/64.0,
                              1.0/256.0, 1.0/64.0, 3.0/128.0, 1.0/64.0, 1.0/256.0};

static const float gaussKernel[9] = {1.0/16.0, 1.0/8.0, 1.0/16.0, 1.0/8.0, 1.0/4.0, 1.0/8.0, 1.0/16.0, 1.0/8.0, 1.0/16.0};

static const int step = 1.0;

static const float sigmaP = 1.0;
static const float sigmaN = 128.0;
static const float sigmaL = 4.0;


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
            float4 moments = gMoment.SampleLevel(gSamPoint, uv + float2(x0, y0) * texelSize, 0);
            float variance = moments.y - moments.x * moments.x;
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
    pixel shader (temporal accumulation)
*/
PS_Output_Acc PS_TemporalAccumulation(PS_Input_Simple ps_input)
{   
    PS_Output_Acc output = (PS_Output_Acc)0;

    float2 uv = ps_input.TexCoord;

    float4 currentColor = gCurrentColor.SampleLevel(gSamPoint, uv, 0);
    float valid;
    float2 prevUV = GetPrevScreenCoordLoad(uv, valid);
    float4 prevColor = gPrevColor.Sample(gSamPoint, prevUV, 0);
    float4 prevMoment = gPrevMoment.Sample(gSamPoint, prevUV, 0);
    float history = prevMoment.w;
    // accumulate history
    history = clamp(lerp(1.0, history + 1, valid), 0, 1024);
    // current luminance
    float newLuma = luma(currentColor.xyz);

    float4 currentMoment = float4(float2(newLuma, newLuma * newLuma), 0, history);

    float alphaColor = max(maxAlphaColor, saturate(1.0 / (history + epsilon)) );
    float alphaMoment = max(maxAlphaMoment, saturate(1.0 / (history + epsilon)) );

    // output accumulated color and moments
    output.Color = float4(lerp(prevColor, currentColor, alphaColor).xyz, history);
    output.Moment = float4(lerp(prevMoment, currentMoment, alphaMoment).xy, 0, history);
    return output;
}


/*
    pixel shader (filter variant)
*/
PS_Output_Simple PS_FilterVariant(PS_Input_Simple ps_input)
{

}


/*
    pixel shader (wavelet filter)
*/
PS_Output_Simple PS_Filter(PS_Input_Simple ps_input)
{   
    PS_Output_Simple output = (PS_Output_Simple)0;

    float2 uv = ps_input.TexCoord;

    // get center pixel
    float3 pColor = gColor.Sample(gSamPoint, uv).xyz;
    float3 pNormal = GetNormal(uv).xyz;
    float3 pPosition = GetPosition(uv);
    float  pObjectId = GetObjectId(uv);
    float  pLuminance = luma(pColor);

    // get texel size
    float2 demension;
    gColor.GetDimensions(demension.x, demension.y);
    float2 texelSize = 1 / demension;


    float  pVariance = GetVariance(uv, texelSize);



    float3 c = float3(0, 0, 0);
    float v = 0.0;
    float weights = 0.0;

    for (int offsetx = -2; offsetx <= 2; offsetx++) {

        for (int offsety = -2; offsety <= 2; offsety++) {

            float2 loc = uv + float2(step * offsetx * texelSize.x, step * offsety * texelSize.y);

            float qObjectId = GetObjectId(loc);

            if (eps_equal(pObjectId, qObjectId)) {

                float3 qPosition = GetPosition(loc).xyz;
                float3 qNormal = GetNormal(loc).xyz;

                float3 qColor = gColor.Sample(gSamPoint, loc).xyz;
               // float qVariance = GetVariance(loc, texelSize);

                float qLuminance = luma(qColor);

                float3 t = pPosition - qPosition;
                float dist2 = dot(t, t) + t.z * t.z;
                float wp = min(exp(-(dist2)/sigmaP), 1.0);

                float wn = pow(max(0.0, dot(pNormal, qNormal)), sigmaN);

                // float gvl = 0.001;
                // for (int y0 = -1; y0 <= 1; y0++) {
                //     for (int x0 = -1; x0 <= 1; x0++) {
                //         gvl += gaussKernel[x0 + 3*y0 + 4] * GetVariance(loc + float2(x0, y0) * texelSize);
                //     }
                // }
                float wl = min(1.0, exp(-abs(pLuminance - qLuminance) / (sigmaL * sqrt(max(0.0, pVariance) + epsilon))));

                float w = wp * wn * wl;
                float weight = h[5*(offsety + 2) + offsetx + 2] * w;

                c += weight * qColor;
               // v += weight * weight * qVariance;
                weights += weight;
            }
        }
    }

    if (weights > epsilon) {
        // output.Color = float4((c / weights).xyz,  v / (weights * weights));
        output.Color = float4((c / weights).xyz,  1);
    } else {
        output.Color = float4(pColor.xyz, 0);
    }
    // output.Color = weights;

   // output.Color.xyz = clamp(output.Color.xyz, float3(0, 0, 0), flaot3(10, 10, 10));
   // cvnext.a = min(max(cvnext.a, 0.0), 10.0);
   // output.Color.w = min(output.Color.w, 10);
   return output;
}

#endif