#ifndef __VSHDR__
#define __VSHDR__

#include "common.fx"

//the most common vertext shader
//using vertex type of vertex_dynamic_instancing
//BTW: every shaders use the same vertex format

static const float3 LumVector  = float3(0.2125f, 0.7154f, 0.0721f);
static const float MiddleGray = 0.2f;
static const float BRIGHT_PASS_THRESHOLD = 5.0f;


struct VS_Input
{
	float3 PosL  : POSITION;
    float2 TexCoord : TEXCOORD;
};

struct PS_Input
{
	float4 PosH : SV_POSITION;
	float2 TexCoord :TEXCOORD0;
};

struct PS_Output
{
	float4 Color : SV_Target0;
};

PS_Input VS(VS_Input input)
{
	PS_Input output = (PS_Input)0;
	output.PosH.xy = 2 * input.PosL.xy - 1;
	output.PosH.zw = 1;
	output.TexCoord = input.TexCoord;
	return output;
}

PS_Output PS_Log(PS_Input input)
{	
	PS_Output output = (PS_Output)0;
	//oColor = (light + 0.1);
	float3 vSample = 0.0f;
	float  fLogLumSum = 0.0f;

    for(int iSample = 0; iSample < 4; iSample++)
    {
        // Compute the sum of log(luminance) throughout the sample points
		vSample = gPostBuffer.Sample(gSamBilinear, input.TexCoord + gSampleOffsets[iSample]);
        fLogLumSum += log(dot(vSample, LumVector)+0.0001f);
    }
    // Divide the sum to complete the average
    fLogLumSum *= 0.25;
	output.Color = float4(fLogLumSum,fLogLumSum,fLogLumSum,0);
	return output;
}

PS_Output PS_Avg(PS_Input input)
{	
	PS_Output output = (PS_Output)0;
	float3  Color = 0.0f;
	float3 vSample = 0.0f;
	for (int iSample = 0; iSample < 4; iSample++)
	{
		// Compute the sum of log(luminance) throughout the sample points
		vSample = gPostBuffer.Sample(gSamBilinear, input.TexCoord + gSampleOffsets[iSample]);
		Color += vSample;
	}
	// Divide the sum to complete the average
	Color *= 0.25;
	output.Color = float4(Color, 0);
	return output;
}

PS_Output PS_Adapt(PS_Input input)
{	
	PS_Output output = (PS_Output)0;
	//oColor = (light + 0.1);
	float OldLum = 0.0f;
	float CurLum = 0.0f;
    OldLum = gDiffuseMap0.Sample(gSam,input.TexCoord);
	float vSample = 0.0f;
	for (int iSample = 0; iSample < 4; iSample++)
	{
		// Compute the sum of log(luminance) throughout the sample points
		vSample = gPostBuffer.Sample(gSamBilinear, input.TexCoord + gSampleOffsets[iSample]);
		CurLum += vSample;
	}
	// Divide the sum to complete the average
	CurLum *= 0.25;
	//vSample = exp(vSample);
	float OutLum = OldLum + (CurLum - OldLum) * ( 1 - pow( 0.98f, 30 * gTimeElapse/1000.0f) );
	output.Color = float4(OutLum, OutLum, OutLum, 0);
	return output;
}

PS_Output PS_ToneMapping(PS_Input input)
{	
	PS_Output output = (PS_Output)0;
	//oColor = (light + 0.1);
	float3 vSample = 0.0f;
	float vLum = 0.0f;
	float4 vBloom = 0.0f;
    vLum = gDiffuseMap0.Sample(gSam,float2(0.5f,0.5f));
	vLum = exp(vLum);
	vSample = gPostBuffer.Sample(gSam,input.TexCoord);
	vBloom =  gDiffuseMap1.Sample(gSam,input.TexCoord);
	vSample.xyz *= MiddleGray /(vLum + 0.001f);
	vSample.xyz /= (1.0f + vSample);

	// bloom effect
	vSample += vBloom * 1.0f;
	output.Color = float4(vSample, 0);
	//output.Color = vLum;
	return output;
}

PS_Output PS_BrightPass(PS_Input input)
{
	PS_Output output = (PS_Output)0;
	float4 vSample = gPostBuffer.Sample(gSam,input.TexCoord);
	float  fAdaptedLum = gDiffuseMap0.Sample(gSam,float2(0.5f,0.5f));;
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
	vSample.rgb /= (10 +vSample);
	output.Color = vSample;
	return output;
}

PS_Output PS_GaussBloom5x5(PS_Input input)
{
	PS_Output output = (PS_Output)0;
	//oColor = (light + 0.1);
	float4 vSample = 0.0f;
    for(int iSample = 0; iSample < 3; iSample++)
    {
        vSample += gSampleWeights[iSample].x * gPostBuffer.Sample(gSamBilinear,input.TexCoord + gSampleOffsets[iSample]);
    }
	output.Color = vSample;
	return output;
}


#endif