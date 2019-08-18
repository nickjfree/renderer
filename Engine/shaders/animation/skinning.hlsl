#include "skinning.h"

/*
    vertex shader for skinning rendering 
*/

PS_Input_GBuffer VS_Skinning_GBuffer(VS_Input_Skinning vs_input) 
{
    // do bone tranformation
    vs_input = transform_bone(vs_input);
    // transform to pixel shader
    PS_Input_GBuffer output;
    output = transform_to_view_gbuffer(vs_input.PosL, vs_input.Normal, 
        vs_input.TexCoord, vs_input.Tangent, gWorldViewProjection, gWorldViewMatrix);
    return output;
}

/*
    vertex shader for skinning rendering simple
*/

PS_Input_Simple VS_Skinning_Simple(VS_Input_Skinning vs_input) 
{
    // do bone tranformation
    vs_input = transform_bone(vs_input);
    // transform to pixel shader
    PS_Input_Simple output;
    output = transform_to_view_simple(vs_input.PosL, vs_input.Normal, 
        vs_input.TexCoord, vs_input.Tangent, gWorldViewProjection, gWorldViewMatrix);
    return output;
}
