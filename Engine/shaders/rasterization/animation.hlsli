#ifndef __ANIMATION__
#define __ANIMATION__

#include "../common/constants.h"

/*
	blendshape vertex
*/
struct Vertex
{
	float3 PosL;
	float3 Normal;
	float2 TexCoord;
#ifdef VS_SKINNING
// skeletion animation
	uint  Bones : TEXCOORD1;
	float3 Weight : TEXCOORD2;
#else
// not used stuff
	float  InstanceID : TEXCOORD1;
#endif
	float3 Tangent;
};

StructuredBuffer<Vertex> gBlendShapes  : register(t1); 

// uav deformable buffer. transformed vertext stored in this buffer
RWStructuredBuffer<Vertex> gDeformableBuffer  : register(u0);


/*
	get shape vertex at vertexId
*/
Vertex get_shape_vertex(uint shape_index, uint vertexId)
{
	uint vertex_addr = vertexId + shape_index * gWeightsArray[0].y;
	Vertex vt = gBlendShapes[vertex_addr];
	return vt;
}

/*
	blendshape transform
*/
Vertex transform_shape(Vertex vsInput, uint vertexId)
{
	// position disposition
	float3 pos_dis = float3(0.0f, 0.0f, 0.0f);
	// loop all the weights
	for (int i=1; i<= gWeightsArray[0].z; i++) {
		uint shape_index = uint(gWeightsArray[i].x);
		float shape_weight = gWeightsArray[i].y;
		Vertex shape_vertex = get_shape_vertex(shape_index, vertexId);
		// lerp position
		pos_dis += (shape_vertex.PosL - vsInput.PosL) * shape_weight;
		// // slerp normal
		// vsInput.Normal =  lerp(vsInput.Normal, shape_vertex.Normal, shape_weight);
		// // slerp tangent
		// vsInput.Normal =  lerp(vsInput.Normal, shape_vertex.Normal, shape_weight);

	}
	vsInput.PosL = vsInput.PosL + pos_dis;
	return vsInput;    
}

#ifdef VS_SKINNING
/*
	bone transformation in local space
*/
Vertex transform_bone(Vertex vsInput)
{
	// get bone ids
	uint4 bones = uint4(
		vsInput.Bones & 0x000000ff,
		vsInput.Bones >> 8 & 0x000000ff,  
		vsInput.Bones >> 16 & 0x000000ff,        
		vsInput.Bones >> 24      
	);
	// weights
	float4 weights = float4(vsInput.Weight.xyz, 0);
	weights.w = 1.0f - (weights.x + weights.y + weights.z);
	// get matrix
	float4x4 mat0 = gSkinMatrix[bones.x];
	float4x4 mat1 = gSkinMatrix[bones.y];
	float4x4 mat2 = gSkinMatrix[bones.z];
	float4x4 mat3 = gSkinMatrix[bones.w];
	// transform position normal and tangent
	float4 position = float4(vsInput.PosL, 1);
	float4 normal = float4(vsInput.Normal, 0);
	float4 tangent = float4(vsInput.Tangent.xyz, 0);

	weights = float4(weights.x, weights.y, weights.z, weights.w);
	// transform position
	position = mul(position, mat0) * weights.x + mul(position, mat1) * weights.y 
		+ mul(position, mat2) * weights.z + mul(position, mat3) * weights.w;
	// transform noraml
	normal = mul(normal, mat0) * weights.x + mul(normal, mat1) * weights.y 
		+ mul(normal, mat2) * weights.z + mul(normal, mat3) * weights.w;
	normal = normalize(normal);
	// transform tangent
	tangent = mul(tangent, mat0) * weights.x + mul(tangent, mat1) * weights.y 
		+ mul(tangent, mat2) * weights.z + mul(tangent, mat3) * weights.w;
	tangent = normalize(tangent);
	// return output
	vsInput.PosL = position.xyz;
	vsInput.Normal = normal.xyz;
	vsInput.Tangent = tangent.xyz;
	return vsInput;
}
#endif


// animated the vertex. blendshape and skinning
Vertex transform_animation(Vertex vsInput, uint vertexId)
{

#ifdef VS_BLENDSHAPE 
	vsInput = transform_shape(vsInput, vertexId);
#endif

#ifdef VS_SKINNING
	vsInput = transform_bone(vsInput);
	// save the deformed mesh
	gDeformableBuffer[vertexId] = vsInput;
#endif
	return vsInput;
}



#endif