
#include "../common/basic_layout.h"


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
	float sacle = input.InstanceScale/8192.0;

	float2 origin = input.InstancePosition.xz - float2(0.5 * input.InstanceScale, 0.5 * input.InstanceScale);
	float2 offset = origin/8192;

    input.PosL.xy = (input.PosL.xy * sacle) + offset;

    output.PosH.xy = 2 * input.PosL.xy - 1;
    output.PosH.zw = 1;
    output.TexCoord = float2(input.InstanceLevel/7, 1 - input.InstanceLevel/7);
    return output;
}

/*
    simple pixel shader (for shadow mapping)
*/

PS_Output_Simple PS(PS_Input_Simple ps_input)
{   
    PS_Output_Simple output = (PS_Output_Simple)0;
    output.Color = float4(ps_input.TexCoord, 0, 0.1);
    return output;
}