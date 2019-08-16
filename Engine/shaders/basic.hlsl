

#include "common/common.h"

/*
    vertex shader for none instancing rendering
*/
PS_Input_GBuffer VS_Basic(VS_Input_Simple vs_input) 
{
    PS_Input_GBuffer output;
    output = transform_to_view_simple(vs_input.PosL, vs_input.Normal, 
        vs_input.TexCoord, vs_input.Tangent, gWorldViewProjection, gWorldViewMatrix);
    return output;
}


/*
    vertex shader for instancing rendering
*/
PS_Input_GBuffer VS_Instancing(VS_Input_Instance vs_input) 
{
    PS_Input_GBuffer output;
    output = transform_to_view_simple(vs_input.PosL, vs_input.Normal, 
        vs_input.TexCoord, vs_input.Tangent, vs_input.InstanceWVP, vs_input.InstanceWV);
    return output;
}
