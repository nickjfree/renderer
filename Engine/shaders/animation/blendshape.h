#ifndef __BLENDSHAPE__
#define __BLENDSHAPE__

#include "common/common.h"


/*
     blendshape parameters
*/

cbuffer ArrayBSParamaters: register(b3)
{
    // weight array, the first elements contains shape description
    float4 gWeightsArray[128]; 
}

#endif
