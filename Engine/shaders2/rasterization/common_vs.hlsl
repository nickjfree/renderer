#ifndef __COMMON_VS__
#define __COMMON_VS__

#include "common.h"


#ifdef SKINNING
    include "../common/animation.hlsli"
#endif

/*
    vertex shader
*/
PS_Input VSMain(VS_Input vs_input) 
{
    PS_Input output;

    // TODO:
    // vs_input = transform_shape(vs_input, VertexId);
#ifdef BLENDSHAPE
    // blendshape
#endif

#ifdef SKINNING
    // skinning 
#endif    

#ifdef defined(BLENDSHAPE) || defined(SKINNING) || defined(CLIPMAP)
    // save deformed vertex to an uav
#endif

    // do rateraztions
#ifdef INSTANCING
    // instancing
    output = transform_to_view(vs_input.PosL, vs_input.Normal, 
        vs_input.TexCoord, vs_input.Tangent, 
        vs_input.InstanceWVP, vs_input.InstanceWV, vs_input.InstancePWVP,
        vs_input.InstanceObjectId);
#else
    // none instancing
    output = transform_to_view(vs_input.PosL, vs_input.Normal, 
        vs_input.TexCoord, vs_input.Tangent, 
        gWorldViewProjection, gWorldViewMatrix, gPrevWorldViewProjection,
        gObjectId);
#endif

    return output;
}

#endif