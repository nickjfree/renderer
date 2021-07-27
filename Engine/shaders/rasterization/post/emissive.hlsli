#ifndef __EMISSIVE__
#define __EMISSIVE__

/*
    render emissive suerface
*/
#include "../../common/gbuffer.hlsli"

// render emissive suerface
float4 ShadePixel(GBufferContext gbuffer)
{
    if (length(gbuffer.ViewSpacePosition) < 0.001) {
        return float4(0, 0, 0, 0);
    }
    float4 diffuse = gbuffer.Diffuse;
    float4 color = diffuse * 100.0f * (1 - diffuse.a);
    return color;
}

#endif
