
// #include "../common/basic_layout.h"
#include "../common/common.h"

// quad vertex
struct VS_Input_Quad_Instance
{
    float3 PosL  : POSITION;
    float2 TexCoord : TEXCOORD;
    float3 InstancePosition : InstancePosition; 
    float InstanceScale: InstanceScale;
    float InstanceLevel: InstanceLevel;
};


// render a screen space quad
PS_Input_Simple VS(VS_Input_Quad_Instance input)
{
    // simple ps shader input layout
    PS_Input_Simple output = (PS_Input_Simple)0;
    // caculate screen space 
    float2 tileScale = float2(0.25f, 0.5f);
	float2 subScale = float2(0.125f, 0.25f);
	float2 origin = input.InstancePosition.xz - float2(0.5 * input.InstanceScale, 0.5 * input.InstanceScale);
    // wrap position
    float2 tileOffset = float2(fmod(input.InstanceLevel, 4), floor(input.InstanceLevel / 4)) * tileScale;
    float2 subOffset = floor(fmod(origin / input.InstanceScale, 2)) * subScale;

	float2 offset = tileOffset + subOffset;

    float2 texCoord = origin +  input.PosL.xy * float2(input.InstanceScale, input.InstanceScale);
    input.PosL.xy = (input.PosL.xy * subScale) + offset;

    output.PosH.xy = 2 * input.PosL.xy - 1;
    output.PosH.zw = 1;
    output.TexCoord = texCoord/8192;
    return output;
}

/*
    simple pixel shader (for shadow mapping)
*/

PS_Output_Simple PS(PS_Input_Simple ps_input)
{   
    PS_Output_Simple output = (PS_Output_Simple)0;
    
    float4 color = gDiffuseMap0.Sample(gSam, ps_input.TexCoord);

    output.Color = float4(color.xyz, 0.9);
    return output;
}