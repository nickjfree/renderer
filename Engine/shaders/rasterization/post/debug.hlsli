#ifndef __EMISSIVE__
#define __EMISSIVE__

/*
    render debug textures
*/
#include "../../common/gbuffer.hlsli"

Texture2D DebugBuffer : register(t0); 

// render emissive suerface
float4 ShadePixel(GBufferContext gbuffer)
{
 	float4 color = DebugBuffer.Sample(gSam, gbuffer.uv);
    return color;
}

#endif
