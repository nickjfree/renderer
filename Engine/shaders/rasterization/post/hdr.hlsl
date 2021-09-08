#ifndef __HDR__
#define __HDR__


#include "../common.hlsli"


static const float3 LumVector  = float3(0.2125f, 0.7154f, 0.0721f);
// static const float  MiddleGray = 0.18f;
static const float  MiddleGray = 0.18f;
static const float  BRIGHT_PASS_THRESHOLD = 10.0f;



Texture2D gDiffuseMap0 : register(t0);
Texture2D gDiffuseMap1 : register(t1);
Texture2D gPostBuffer : register(t2);

/*
    pixel shader get averaged log Lum value
*/
PS_Output PS_Log(PS_Input input)
{   
    PS_Output output = (PS_Output)0;
    //oColor = (light + 0.1);
    float3 vSample = 0.0f;
    float  fLogLumSum = 0.0f;

    for(int iSample = 0; iSample < 4; iSample++)
    {
        // Compute the sum of log(luminance) throughout the sample points
        vSample = gPostBuffer.Sample(gSamBilinear, input.TexCoord + gSampleOffsets[iSample].xy).xyz;
        fLogLumSum += log(dot(vSample, LumVector)+0.0001f);
        // fLogLumSum += dot(vSample, LumVector);
    }
    // Divide the sum to complete the average
    fLogLumSum *= 0.25;
    // fLogLumSum = log(fLogLumSum + 0.0001f);
    output.Color = float4(fLogLumSum, fLogLumSum, fLogLumSum,0);
    return output;
}


/*
    down sampling pixel shader
*/
PS_Output PS_Avg(PS_Input input)
{   
    PS_Output output = (PS_Output)0;
    float3  Color = 0.0f;
    float3 vSample = 0.0f;
    for (int iSample = 0; iSample < 4; iSample++)
    {
        // Compute the sum of log(luminance) throughout the sample points
        vSample = gPostBuffer.Sample(gSamBilinear, input.TexCoord + gSampleOffsets[iSample].xy).xyz;
        Color += vSample;
    }
    // Divide the sum to complete the average
    Color *= 0.25;
    output.Color = float4(Color, 0);
    return output;
}

/*
    eye adaptation pixel shader
*/
PS_Output PS_Adapt(PS_Input input)
{   
    PS_Output output = (PS_Output)0;
    //oColor = (light + 0.1);
    float OldLum = 0.0f;
    float CurLum = 0.0f;
    OldLum = gDiffuseMap0.Sample(gSam,input.TexCoord).x;
    if (((asuint(OldLum) & 0x7f800000u) == 0x7f800000u) && (asuint(OldLum) & 0x7fffffu) ) {
        OldLum = 0;
    }
    float vSample = 0.0f;
    for (int iSample = 0; iSample < 4; iSample++)
    {
        // Compute the sum of log(luminance) throughout the sample points
        vSample = gPostBuffer.Sample(gSamBilinear, input.TexCoord + gSampleOffsets[iSample].xy).x;
        CurLum += vSample;
    }
    // Divide the sum to complete the average
    CurLum *= 0.25;
    //vSample = exp(vSample);
    float OutLum = OldLum + (CurLum - OldLum) * ( 1 - pow( 0.98f, 30 * gTimeElapse/1000.0f) );
    output.Color = float4(OutLum, OutLum, OutLum, 0);
    return output;
}

/*
    tonemapping
*/
PS_Output PS_ToneMapping(PS_Input input)
{   
    PS_Output output = (PS_Output)0;
    //oColor = (light + 0.1);
    float3 vSample = 0.0f;
    float vLum = 0.0f;
    float4 vBloom = 0.0f;
    vLum = gDiffuseMap0.Sample(gSam,float2(0.5f, 0.5f)).x;
    vLum = exp(vLum);
    vSample = gPostBuffer.Sample(gSam, input.TexCoord).xyz;
    vBloom =  gDiffuseMap1.Sample(gSam, input.TexCoord);
    vSample.xyz *= MiddleGray /(vLum + 0.001f);
    vSample.xyz /= (1.0f + vSample);

    // bloom effect
    vSample += vBloom.xyz * 0.3f;
    output.Color = float4(vSample, 0);
    return output;
}


/*
    bright pass
*/
PS_Output PS_BrightPass(PS_Input input)
{
    PS_Output output = (PS_Output)0;
    float4 vSample = gPostBuffer.Sample(gSam, input.TexCoord);
    float  fAdaptedLum = gDiffuseMap0.Sample(gSam, float2(0.5f,0.5f)).x;
    fAdaptedLum = exp(fAdaptedLum);
    // Determine what the pixel's value will be after tone-mapping occurs
    vSample.rgb *= MiddleGray /(fAdaptedLum + 0.001f);
    
    // Subtract out dark pixels
    vSample.rgb -= BRIGHT_PASS_THRESHOLD;
    
    // Clamp to 0
    vSample = max(vSample, 0.0f);
    
    // Map the resulting value into the 0 to 1 range. Higher values for
    // BRIGHT_PASS_OFFSET will isolate lights from illuminated scene 
    // objects.
    vSample.rgb /= (10 + vSample.rgb);
    output.Color = vSample;
    return output;
}

/*
    GaussBloom5x5
*/
PS_Output PS_GaussBloom5x5(PS_Input input)
{
    PS_Output output = (PS_Output)0;
    //oColor = (light + 0.1);
    float4 vSample = 0.0f;
    for(int iSample = 0; iSample < 3; iSample++)
    {
        vSample += gSampleWeights[iSample].x * gPostBuffer.Sample(gSamBilinear, input.TexCoord + gSampleOffsets[iSample].xy);
    }
    output.Color = vSample;
    return output;
}

#endif