#ifndef __COMMON_VS__
#define __COMMON_VS__



#include "common.hlsli"


/*
	uniform vertex shader
*/
PS_Input VSMain(VS_Input vsInput, uint vertexId : SV_VertexId) 
{
	PS_Input output = (PS_Input)0;;

	// TODO:
	// vs_input = transform_shape(vs_input, VertexId);
#if defined(VS_BLENDSHAPE) || defined(VS_SKINNING) 
	// blendshape and skinning tranform
	vsInput = transform_animation(vsInput, vertexId);
#endif

#ifdef VS_CLIPMAP
	// terrain clipmap rendering
	vsInput = transform_terrain(vsInput, vertexId);
#endif

	// do rateraztions
#ifdef VS_INSTANCING
	// instancing
	output = transform_to_view(vsInput.PosL, vsInput.Normal, 
		vsInput.TexCoord, vsInput.Tangent, 
		vsInput.InstanceWVP, vsInput.InstanceWV, vsInput.InstancePWVP,
		vsInput.InstanceObjectId);

#elif RENDER_SCREEN

	// caculate screen space
	output.PosH.xy = 2 * vsInput.PosL.xy - 1;
	output.PosH.zw = 1;
	output.TexCoord = vsInput.TexCoord;
#elif RENDER_DEBUG
	// small quad
	output.PosH.xy = 2 * vsInput.PosL.xy * 0.125 - 1;
	output.PosH.zw = 1;
	output.TexCoord = vsInput.TexCoord;
#else
	// gbuffer
	output = transform_to_view(vsInput.PosL, vsInput.Normal, 
		vsInput.TexCoord, vsInput.Tangent, 
		gWorldViewProjection, gWorldViewMatrix, gPrevWorldViewProjection,
		gObjectId);
#endif

	return output;
}

#endif