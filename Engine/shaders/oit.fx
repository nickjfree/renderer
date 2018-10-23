// Copyright 2011 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.

#ifndef H_AOIT
#define H_AOIT

typedef uint COLOR;

float UnpackUnnormAlpha(COLOR packedInput)
{
    return (float)(packedInput >> 24UL);
}

float3 UnpackRGB(COLOR packedInput) 
{
    float3 unpackedOutput;
    uint3 p = uint3((packedInput & 0xFFUL),
                    (packedInput >> 8UL) & 0xFFUL,
                    (packedInput >> 16UL) & 0xFFUL);

    unpackedOutput = ((float3)p) / 255;
    return unpackedOutput;
}

float4 UnpackRGBA(COLOR packedInput)
{
    float4 unpackedOutput;
    uint4 p = uint4((packedInput & 0xFFUL),
                    (packedInput >> 8UL) & 0xFFUL,
                    (packedInput >> 16UL) & 0xFFUL,
                    (packedInput >> 24UL));

    unpackedOutput = ((float4)p) / 255;
    return unpackedOutput;
}

COLOR PackRGB(float3 unpackedInput)
{
    uint3 u = (uint3)(saturate(unpackedInput) * 255 + 0.5);
    uint  packedOutput = (u.z << 16UL) | (u.y << 8UL) | u.x;
    return packedOutput;
}

COLOR PackRGBA(float4 unpackedInput)
{
    uint4 u = (uint4)(saturate(unpackedInput) * 255 + 0.5);
    uint  packedOutput = (u.w << 24UL) | (u.z << 16UL) | (u.y << 8UL) | u.x;
    return packedOutput;
}



//////////////////////////////////////////////
// Defines
//////////////////////////////////////////////

#ifdef aoit_node_count
#define AOIT_NODE_COUNT  aoit_node_count
#endif


#ifndef AOIT_NODE_COUNT 
#define AOIT_NODE_COUNT         (4)
#endif

#if AOIT_NODE_COUNT == 2
#define AOIT_RT_COUNT           (1)
#else
#define AOIT_RT_COUNT           (AOIT_NODE_COUNT / 4)
#endif

// Forces compression to only work on the second half of the nodes (cheaper and better IQ in some cases)

#if AOIT_NODE_COUNT >= 8
#define AOIT_DONT_COMPRESS_FIRST_HALF 
#endif

#define AOIT_EMPTY_NODE_DEPTH   (1E30f)

//#define AOIT_EARLY_Z_CULL

#define AOIT_TILED_ADDRESSING

//////////////////////////////////////////////
// Structs
//////////////////////////////////////////////

struct AOITCtrlSurface
{
    bool  clear;
    bool  opaque;
    float depth;
};

struct AOITSPData
{
    float4 depth[AOIT_RT_COUNT];
    uint4  color[AOIT_RT_COUNT];
};

struct AOITSPDepthData
{
    float4 depth[AOIT_RT_COUNT];
};

struct AOITSPColorData
{
    uint4  color[AOIT_RT_COUNT];
};

struct ATSPNode
{
    float  depth;
    float  trans;
    uint   color;
};

//////////////////////////////////////////////
// Resources
//////////////////////////////////////////////


#if AOIT_NODE_COUNT == 8
#define _AOITSPDepthDataUAV g8AOITSPDepthDataUAV
#define _AOITSPColorDataUAV g8AOITSPColorDataUAV
#define _AOITSPColorDataSRV g8AOITSPColorDataSRV
#else
#define _AOITSPDepthDataUAV gAOITSPDepthDataUAV
#define _AOITSPColorDataUAV gAOITSPColorDataUAV
#define _AOITSPColorDataSRV gAOITSPColorDataSRV
#endif

// Since there's no reflection on the cpp side for these, set registers explicitly - don't change them, this is the expected order
RWTexture2D<uint> gAOITSPClearMaskUAV                       : register( u1 );
RWStructuredBuffer<AOITSPDepthData> _AOITSPDepthDataUAV     : register( u2 );
RWStructuredBuffer<AOITSPColorData> _AOITSPColorDataUAV     : register( u3 );

Texture2D<uint> gAOITSPClearMaskSRV;
StructuredBuffer<AOITSPColorData> _AOITSPColorDataSRV;


//////////////////////////////////////////////
// Main AOIT fragment insertion code
//////////////////////////////////////////////

//////////////////////////////////////////////
// Main AOIT fragment insertion code
//////////////////////////////////////////////

void AOITSPInsertFragment(in float  fragmentDepth,
                          in float  fragmentTrans,
                          in float3 fragmentColor,
                          inout ATSPNode nodeArray[AOIT_NODE_COUNT])
{   
    int i, j;

    float  depth[AOIT_NODE_COUNT + 1];  
    float  trans[AOIT_NODE_COUNT + 1];   
    uint   color[AOIT_NODE_COUNT + 1];   

    ///////////////////////////////////////////////////
    // Unpack AOIT data
    ///////////////////////////////////////////////////                   
    [unroll] for (i = 0; i < AOIT_NODE_COUNT; ++i) {
        depth[i] = nodeArray[i].depth;
        trans[i] = nodeArray[i].trans;
        color[i] = nodeArray[i].color;
    }   
    
    // Find insertion index 
    int index = 0;
    float prevTrans = 255;
    [unroll] for (i = 0; i < AOIT_NODE_COUNT; ++i) {
        if (fragmentDepth > depth[i]) {
            index++;
            prevTrans = trans[i];
        }
    }

    // Make room for the new fragment. Also composite new fragment with the current curve 
    // (except for the node that represents the new fragment)
    [unroll]for (i = AOIT_NODE_COUNT - 1; i >= 0; --i) {
        [flatten]if (index <= i) {
            depth[i + 1] = depth[i];
            trans[i + 1] = trans[i] * fragmentTrans;
            color[i + 1] = color[i];
        }
    }
    
    // Insert new fragment
    const float newFragTrans = fragmentTrans * prevTrans;
    const uint  newFragColor = PackRGB(fragmentColor * (1 - fragmentTrans));
    [unroll]for (i = 0; i <= AOIT_NODE_COUNT; ++i) {
        [flatten]if (index == i) {
            depth[i] = fragmentDepth;
            trans[i] = newFragTrans;
            color[i] = newFragColor;
        }
    } 

    // pack representation if we have too many nodes
    [flatten]if (depth[AOIT_NODE_COUNT] != AOIT_EMPTY_NODE_DEPTH) {
        float3 toBeRemovedCol = UnpackRGB(color[AOIT_NODE_COUNT]);
        float3 toBeAccumulCol = UnpackRGB(color[AOIT_NODE_COUNT - 1]);
        color[AOIT_NODE_COUNT - 1] = PackRGB(toBeAccumulCol + toBeRemovedCol * trans[AOIT_NODE_COUNT - 1] * 
                                             rcp(trans[AOIT_NODE_COUNT - 2]));
        trans[AOIT_NODE_COUNT - 1] = trans[AOIT_NODE_COUNT];
    }
   
    // Pack AOIT data
    [unroll] for (i = 0; i < AOIT_NODE_COUNT; ++i) {
        nodeArray[i].depth = depth[i];
        nodeArray[i].trans = trans[i];
        nodeArray[i].color = color[i];
    }
}

/////////////////////////////////////////////////
// Address generation functions for the AOIT data
/////////////////////////////////////////////////

uint AOITAddrGen(uint2 addr2D, uint surfaceWidth)
{
#ifdef AOIT_TILED_ADDRESSING

    surfaceWidth      = surfaceWidth >> 1U;
    uint2 tileAddr2D  = addr2D >> 1U;
    uint  tileAddr1D  = (tileAddr2D[0] + surfaceWidth * tileAddr2D[1]) << 2U;
    uint2 pixelAddr2D = addr2D & 0x1U;
    uint  pixelAddr1D = (pixelAddr2D[1] << 1U) + pixelAddr2D[0];
    
    return tileAddr1D | pixelAddr1D;
#else
    return addr2D[0] + surfaceWidth * addr2D[1];    
#endif
}

uint AOITAddrGenUAV(uint2 addr2D)
{
    uint2 dim;
    gAOITSPClearMaskUAV.GetDimensions(dim[0], dim[1]);
    return AOITAddrGen(addr2D, dim[0]);
}

uint AOITAddrGenSRV(uint2 addr2D)
{
    uint2 dim;
    gAOITSPClearMaskSRV.GetDimensions(dim[0], dim[1]);
    return AOITAddrGen(addr2D, dim[0]); 
}


void AOITSPClearData(inout AOITSPData data, float depth, float4 color)
{
    uint packedColor = PackRGBA(float4(0, 0, 0, 1.0f - color.w));

#if AOIT_NODE_COUNT == 2
    data.depth[0] = 0;
    data.color[0][0] = asuint(depth);
    data.color[0][1] = asuint((float)AOIT_EMPTY_NODE_DEPTH);
    data.color[0][2] = PackRGBA(float4(color.www * color.xyz, 1.0f - color.w));
    data.color[0][3] = packedColor;
#else
    [unroll]for(uint i = 0; i < AOIT_RT_COUNT; i++) {
        data.depth[i] = AOIT_EMPTY_NODE_DEPTH;
        data.color[i] = packedColor;
    }   
    data.depth[0][0] = depth;
    data.color[0][0] = PackRGBA(float4(color.www * color.xyz, 1.0f - color.w));
#endif
}

/////////////////////////////////////////////////
// Load/store functions for the AOIT data
/////////////////////////////////////////////////

void AOITSPLoadDataSRV(in uint2 pixelAddr, out ATSPNode nodeArray[AOIT_NODE_COUNT])
{
    AOITSPData data;
    uint addr  = AOITAddrGenSRV(pixelAddr);
    data.color = _AOITSPColorDataSRV[addr];

#if AOIT_NODE_COUNT == 2
    [unroll]for(uint j = 0; j < 2; j++) {
        ATSPNode node = { 0, UnpackUnnormAlpha(data.color[0][2 + j]), 
                             data.color[0][2 + j] & 0xFFFFFFUL };
        nodeArray[j] = node;
    }
#else
    [unroll]for(uint i = 0; i < AOIT_RT_COUNT; i++) {
        [unroll]for(uint j = 0; j < 4; j++) {
            ATSPNode node = { 0, UnpackUnnormAlpha(data.color[i][j]), 
                                 data.color[i][j] & 0xFFFFFFUL };
            nodeArray[4 * i + j] = node;
        }
    }
#endif
}

void AOITSPLoadDataUAV(in uint2 pixelAddr, out ATSPNode nodeArray[AOIT_NODE_COUNT])
{
    AOITSPData data;    
    uint addr  = AOITAddrGenUAV(pixelAddr);
    data.color = _AOITSPColorDataUAV[addr];

#if AOIT_NODE_COUNT == 2
    [unroll]for(uint j = 0; j < 2; j++) {
        ATSPNode node = { asfloat(data.color[0][j]), 
                          UnpackUnnormAlpha(data.color[0][2 + j]), 
                          data.color[0][2 + j] & 0xFFFFFFUL };
        nodeArray[j] = node;
    }
#else
    data.depth = _AOITSPDepthDataUAV[addr];
    [unroll]for(uint i = 0; i < AOIT_RT_COUNT; i++) {
        [unroll]for(uint j = 0; j < 4; j++) {
            ATSPNode node = { data.depth[i][j], 
                              UnpackUnnormAlpha(data.color[i][j]), 
                              data.color[i][j] & 0xFFFFFFUL };
            nodeArray[4 * i + j] = node;
        }
    }   
#endif
}

void AOITSPStoreDataUAV(in uint2 pixelAddr, ATSPNode nodeArray[AOIT_NODE_COUNT])
{
    AOITSPData data;
    uint addr = AOITAddrGenUAV(pixelAddr);

#if AOIT_NODE_COUNT == 2
    [unroll]for(uint j = 0; j < 2; j++) {
        data.color[0][j] =  asuint(nodeArray[j].depth);
        data.color[0][2 + j] = (nodeArray[j].color & 0xFFFFFFUL) | 
                               (((uint)(nodeArray[j].trans)) << 24UL);
    }
#else
    [unroll]for(uint i = 0; i < AOIT_RT_COUNT; i++) {
        [unroll]for(uint j = 0; j < 4; j++) {
            data.depth[i][j] =  nodeArray[4 * i + j].depth;
            data.color[i][j] = (nodeArray[4 * i + j].color & 0xFFFFFFUL) | 
                               (((uint)(nodeArray[4 * i + j].trans)) << 24UL);
        }
    }
    _AOITSPDepthDataUAV[addr] = data.depth;
#endif

    _AOITSPColorDataUAV[addr] = data.color;
}

/////////////////////////////////////////////////////////////
// Control Surface functions for the AOIT data
// We use this surface to remove the overhead incurred in 
// clearing large AOIT buffers by storing for each
// pixel on the screen a to-be-cleared flag.
// We use the same structure to store some additional
// per-pixel information such as the depth of the most
// distant transparent fragment and its total transmittance,
// which in turn can be used to perform early-z culling over
// pixels covered by transparent fragments
/////////////////////////////////////////////////////////////

void AOITLoadControlSurface(in uint data, inout AOITCtrlSurface surface)
{
    surface.clear   = data & 0x1 ? true : false;
    surface.opaque  = data & 0x2 ? true : false;
    surface.depth   = asfloat((data & 0xFFFFFFFCUL) | 0x3UL);
}

void AOITLoadControlSurfaceUAV(in uint2 pixelAddr, inout AOITCtrlSurface surface)
{
    uint data = gAOITSPClearMaskUAV[pixelAddr];
    AOITLoadControlSurface(data, surface);
}

void AOITLoadControlSurfaceSRV(in uint2 pixelAddr, inout AOITCtrlSurface surface)
{
    uint data = gAOITSPClearMaskSRV[pixelAddr];
    AOITLoadControlSurface(data, surface);
}

void AOITStoreControlSurface(in uint2 pixelAddr, in AOITCtrlSurface surface)
{
    uint data;
    data  = asuint(surface.depth) & 0xFFFFFFFCUL;
    data |= surface.opaque ? 0x2 : 0x0;
    data |= surface.clear  ? 0x1 : 0x0;  
    gAOITSPClearMaskUAV[pixelAddr] = data;
}

void WriteNewPixelToAOIT(float2 Position, float  surfaceDepth, float4 surfaceColor)
{   
    // From now on serialize all UAV accesses (with respect to other fragments shaded in flight which map to the same pixel)
#ifdef dopso
    IntelExt_Init();
    IntelExt_BeginPixelShaderOrdering();
#endif
    ATSPNode nodeArray[AOIT_NODE_COUNT];    
    uint2 pixelAddr = uint2(Position.xy);


    // Load AOIT control surface
    AOITCtrlSurface ctrlSurface;
    AOITLoadControlSurfaceUAV(pixelAddr, ctrlSurface);

    // If we are modifying this pixel for the first time we need to clear the AOIT data
    if (ctrlSurface.clear) 
    {           
        // Clear AOIT data and initialize it with first transparent layer
        AOITSPData data;
        AOITSPClearData(data, surfaceDepth, surfaceColor);          

        // Store AOIT data
        uint addr = AOITAddrGenUAV(pixelAddr);
#if AOIT_NODE_COUNT != 2
        _AOITSPDepthDataUAV[addr] = data.depth;
#endif
        _AOITSPColorDataUAV[addr] = data.color;
            
        // Update control surface
        // ( depth and opaque flag can be used to branch out early if adding behind already near-opaque contents of AOIT )
        // ctrlSurface.clear  = false;          
        // ctrlSurface.opaque = false; // 1.f == surfaceColor.w;
        // ctrlSurface.depth  = 0; // surfaceDepth;
        // AOITStoreControlSurface(pixelAddr, ctrlSurface);

        gAOITSPClearMaskUAV[pixelAddr] = 0;
    } 
    else 
    { 
        // Load AOIT data
        AOITSPLoadDataUAV(pixelAddr, nodeArray);

        // Update AOIT data
        AOITSPInsertFragment(surfaceDepth,      
                             1.0f - surfaceColor.w,  // transmittance = 1 - alpha
                             surfaceColor.xyz,
                             nodeArray);
        // Store AOIT data
        AOITSPStoreDataUAV(pixelAddr, nodeArray);
    }
}

#endif // H_AOIT

