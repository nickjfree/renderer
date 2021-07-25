#ifndef __SS_PS__
#define __SS_PS__


/*
*	common screen space shader
*/


#include "common.hlsli"
#include "../common/gbuffer.hlsli"

#ifdef IBL
	#include "../common/ibl.hlsli"
	#define ShadePixel ShadePixelIBL
#endif 

#ifdef EMISSIVE
	#include "post/emissive.hlsli"
#endif

#ifdef SSAO
	#include "post/ssao.hlsli"
#endif

#ifdef RESOLVE
	#include "post/resolve.hlsli"
#endif

/*
	uniform pixel shader
*/
PS_Output PSMain(PS_Input psInput)
{   
	PS_Output output = (PS_Output)0;

	GBufferContext gbuffer = GetGBufferContext(psInput.TexCoord.xy);
	output.Color = ShadePixel(gbuffer);

	return output;
}








#endif