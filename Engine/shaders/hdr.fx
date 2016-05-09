#ifndef __VSHDR__
#define __VSHDR__

#include "common.fx"

//the most common vertext shader
//using vertex type of vertex_dynamic_instancing
//BTW: every shaders use the same vertex format

static const float3 LumVector  = float3(0.2125f, 0.7154f, 0.0721f);


struct VS_Input
{
	float3 PosL  : POSITION;
    float4 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    float  InstanceID : TEXCOORD1;
    float4 Tangent  : TANGENT;
};

struct PS_Input
{
	float4 PosH : SV_POSITION;
	float4 Normal : NORMAL;
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

PS_Output PS(PS_Input input)
{	
	PS_Output output = (PS_Output)0;
	//oColor = (light + 0.1);
	float3 vSample = 0.0f;
	float  fLogLumSum = 0.0f;

    for(int iSample = 0; iSample < 16; iSample++)
    {
        // Compute the sum of log(luminance) throughout the sample points
        vSample = gFinalBuffer.Sample(gSam,input.TexCoord  + gSampleOffsets[iSample]);
		//vSample = tex2D(gFinalBuffer,input.TexCoord  + gSampleOffsets[iSample]);
        fLogLumSum += dot(vSample, LumVector);
    }
    // Divide the sum to complete the average
    fLogLumSum /= 16;
	output.Color = float4(fLogLumSum,fLogLumSum,fLogLumSum,0);
//	output.Color = float4(gSampleOffsets[2],0,0);
	return output;
}

PS_Output PS_Scale(PS_Input input)
{	
	PS_Output output = (PS_Output)0;
	//oColor = (light + 0.1);
	float3 vSample = 0.0f;
    for(int iSample = 0; iSample < 16; iSample++)
    {
        // Compute the sum of log(luminance) throughout the sample points
        vSample += gDiffuseMap0.Sample(gSam,input.TexCoord + gSampleOffsets[iSample]);
    }
    // Divide the sum to complete the average
    vSample /= 16.0f;
    // Divide the sum to complete the average
	output.Color = float4(vSample,0);
	return output;
}

PS_Output PS_Test(PS_Input input)
{	
	PS_Output output = (PS_Output)0;
	//oColor = (light + 0.1);
	float3 vSample = 0.0f;
    vSample = gDiffuseMap0.Sample(gSam,input.TexCoord);
    // Divide the sum to complete the average
    // Divide the sum to complete the average
	//vSample = exp(vSample);
	output.Color = float4(vSample,0);
	return output;
}

PS_Output PS_Adapt(PS_Input input)
{	
	PS_Output output = (PS_Output)0;
	//oColor = (light + 0.1);
	float OldLum = 0.0f;
	float CurLum = 0.0f;
    OldLum = gDiffuseMap0.Sample(gSam,input.TexCoord);
	CurLum = gDiffuseMap1.Sample(gSam,input.TexCoord);
    // Divide the sum to complete the average
    // Divide the sum to complete the average
	//vSample = exp(vSample);
	float OutLum = OldLum + (CurLum - OldLum) * ( 1 - pow( 0.98f, 30 * gTimeElapse/1000.0f) );
//	OutLum = CurLum;
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
	vSample = gFinalBuffer.Sample(gSam,input.TexCoord);
	vBloom =  gDiffuseMap1.Sample(gSam,input.TexCoord);
	vSample.xyz *= 1.0f/(vLum + 0.001f);
	vSample.xyz /= (1.0f + vSample);

	// bloom effect
	vSample += vBloom * 1.0f;
	output.Color = float4(vSample, 0);
	//output.Color = vBloom;
	return output;
}

PS_Output PS_BrightPass(PS_Input input)
{
	PS_Output output = (PS_Output)0;
	float4 vSample = gFinalBuffer.Sample(gSam,input.TexCoord);
	float  fAdaptedLum = gDiffuseMap0.Sample(gSam,float2(0.5f,0.5f));;
	
	// Determine what the pixel's value will be after tone-mapping occurs
	vSample.rgb *= 0.25f/(fAdaptedLum + 0.001f);
	
	// Subtract out dark pixels
	vSample.rgb -= 1;
	
	// Clamp to 0
	vSample = max(vSample, 0.0f);
	
	// Map the resulting value into the 0 to 1 range. Higher values for
	// BRIGHT_PASS_OFFSET will isolate lights from illuminated scene 
	// objects.
	vSample.rgb /= (5+vSample);
	output.Color = vSample;
	return output;
}

PS_Output PS_GaussBloom5x5(PS_Input input)
{
	PS_Output output = (PS_Output)0;
	//oColor = (light + 0.1);
	float4 vSample = 0.0f;
    for(int iSample = 0; iSample < 13; iSample++)
    {
        // Compute the sum of log(luminance) throughout the sample points
        vSample += gSampleWeights[iSample] * gDiffuseMap0.Sample(gSam,input.TexCoord + gSampleOffsets[iSample]);
    }
	output.Color = vSample;
	return output;
}


PS_Output PS_GaussBloom(PS_Input input)
{
	PS_Output output = (PS_Output)0;
	//oColor = (light + 0.1);
	float4 vSample = 0.0f;
    for(int iSample = 0; iSample < 15; iSample++)
    {
        // Compute the sum of log(luminance) throughout the sample points
        vSample += gSampleWeights[iSample] * gDiffuseMap0.Sample(gSam,input.TexCoord + gSampleOffsets[iSample]);
    }
	output.Color = vSample;
	return output;
}


#endif