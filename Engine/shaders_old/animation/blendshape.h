#ifndef __BLENDSHAPE__
#define __BLENDSHAPE__

#include "../common/common.h"

/*
    vertex layout 
*/
struct Vertex
{
    float3 PosL;
    float3 Normal;
    float2 TexCoord;
    float  InstanceID;
    float3 Tangent;
};

/*
    blendshapes
*/
StructuredBuffer<Vertex> gBlendShapes  : register( t1 ); 

/*
     blendshape parameters
*/

cbuffer ArrayBSParamaters: register(b3)
{
    // weight array, the first elements contains shape description, followed with weights 
    float4 gWeightsArray[128]; 
    // gWeightsArray[0].x: shape count
    // gWeightsArray[0].y: shape stride
    // gWeightsArray[0].z: num weights
    // gWeightsArray[0].w: padding
    // gWeightsArray[1]: weights 0
    // gWeightsArray[2]: weights 1
    // gWeightsArray[3]: weights 2
    // .... 
    // gWeightsArray[n]: weights n-1
}




/*
    get shape vertex at VertexId
*/
Vertex get_shape_vertex(uint shape_index, uint VertexId)
{
    uint vertex_addr = VertexId + shape_index * gWeightsArray[0].y;
    Vertex vt = gBlendShapes[vertex_addr];
    return vt;
}



/*
    blendshape transform
*/
VS_Input_Simple transform_shape(VS_Input_Simple vs_input, uint VertexId)
{
    // position disposition
    float3 pos_dis = float3(0.0f, 0.0f, 0.0f);
    // loop all the weights
    for (int i=1; i<= gWeightsArray[0].z; i++) {
        uint shape_index = uint(gWeightsArray[i].x);
        float shape_weight = gWeightsArray[i].y;
        Vertex shape_vertex = get_shape_vertex(shape_index, VertexId);
        // lerp position
        pos_dis += (shape_vertex.PosL - vs_input.PosL) * shape_weight;
        // // slerp normal
        // vs_input.Normal =  lerp(vs_input.Normal, shape_vertex.Normal, shape_weight);
        // // slerp tangent
        // vs_input.Normal =  lerp(vs_input.Normal, shape_vertex.Normal, shape_weight);

    }
    vs_input.PosL = vs_input.PosL + pos_dis;
    return vs_input;    
}


#endif
