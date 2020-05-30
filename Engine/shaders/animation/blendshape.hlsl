#include "blendshape.h"

/*
    vertex shader for skinning rendering 
*/

PS_Input_GBuffer VS_BlendShape_GBuffer(VS_Input_Simple vs_input, uint VertexId : SV_VertexId) 
{
    // do bone tranformation
    vs_input = transform_shape(vs_input, VertexId);
    // transform to pixel shader
    PS_Input_GBuffer output;
    output = transform_to_view_gbuffer(vs_input.PosL, vs_input.Normal, 
        vs_input.TexCoord, vs_input.Tangent, gWorldViewProjection, gWorldViewMatrix, gPrevWorldViewProjection);
    return output;
}

/*
    vertex shader for skinning rendering simple
*/

PS_Input_Simple VS_BlendShape_Simple(VS_Input_Simple vs_input, uint VertexId : SV_VertexId) 
{
    // do bone tranformation
    vs_input = transform_shape(vs_input, VertexId);
    // transform to pixel shader
    PS_Input_Simple output;
    output = transform_to_view_simple(vs_input.PosL, vs_input.Normal, 
        vs_input.TexCoord, vs_input.Tangent, gWorldViewProjection, gWorldViewMatrix);
    return output;
}
