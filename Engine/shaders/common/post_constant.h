#ifndef  __POST_CONSTANT__
#define  __POST_CONSTANT__

/*
    post processing constant buffer definition
*/

cbuffer Misc : register(b4)
{
    float4 gScreenSize;
    float4x4 pad1;
}

#endif