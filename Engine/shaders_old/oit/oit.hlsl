#include "../common/common.h"
#include "oit.h"

//the most common vertext shader
//using vertex type of vertex_dynamic_instancing
//BTW: every shaders use the same vertex format

/*
    a-buffer buiding pass  oit-init
*/
[earlydepthstencil]
PS_Output_Simple PS(PS_Input_Simple input)
{   
    PS_Output_Simple output = (PS_Output_Simple)0;
    float4 diffuse = gDiffuseMap0.Sample(gSam, input.TexCoord);
    float3 color = diffuse.xyz;
    output.Color = float4(color, 0.5);
    WriteNewPixelToAOIT(input.PosH.xy, 0.5f, output.Color);
    return output;
}


struct VS_Input
{
    float3 PosL  : POSITION;
    float2 TexCoord : TEXCOORD;
};

struct PS_Input
{
    float4 PosH : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

PS_Input VS(VS_Input input)
{
    PS_Input output = (PS_Input)0;
    output.PosH.xy = 2 * input.PosL.xy - 1;
    output.PosH.zw = 1;
    output.TexCoord = input.TexCoord;
    return output;
}

/*
    oit resolv pass
*/
float4 AOITSPResolvePS(float4 pos: SV_POSITION, float2 tex : TEXCOORD0) : SV_Target
{
    float4 outColor = float4(0, 0, 0, 1);
    uint2 pixelAddr = uint2(pos.xy);

    // display debug colour
    //return float4( 0, 1, 0, 1.0 );

    // Load control surface
    AOITCtrlSurface ctrlSurface;
    AOITLoadControlSurfaceSRV(pixelAddr, ctrlSurface);

    // Any transparent fragment contributing to this pixel?
    // if (!ctrlSurface.clear) 

    if (!ctrlSurface.clear) 
    {
        // Load all nodes for this pixel    
        ATSPNode nodeArray[AOIT_NODE_COUNT];
        AOITSPLoadDataSRV(pixelAddr, nodeArray);

        // Accumulate final transparent colors
        float  trans = 1;
        float3 color = 0;       
        [unroll]for(uint i = 0; i < AOIT_NODE_COUNT; i++) {
#ifdef dohdr
            color += trans * FromRGBE(UnpackRGBA(nodeArray[i].color));
#else
            color += trans * UnpackRGB(nodeArray[i].color);
#endif
            trans  = nodeArray[i].trans / 255;
        }
        outColor = float4(color, nodeArray[AOIT_NODE_COUNT - 1].trans / 255);
    }

    // blend accumualted transparent color with opaque background color
    return outColor;
}


/*
    OIT Clear Pass
*/
[earlydepthstencil]
void AOITSPClearPS( float4 pos: SV_POSITION, float2 tex : TEXCOORD0 )
{
    uint2 pixelAddr = uint2(pos.xy);

    uint addr = AOITAddrGenUAV(pixelAddr);

    uint data = 0x1; // is clear
    gAOITSPClearMaskUAV[pixelAddr] = data;
}
