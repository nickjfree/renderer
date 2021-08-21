#ifndef __SHADER_BINDING_MAP__
#define __SHADER_BINDING_MAP__



/*
*	material 
*/
#define SLOT_MATERIAL_DIFFUSE0   0
#define SLOT_MATERIAL_DIFFUSE1 	 1
#define SLOT_MATERIAL_DIFFUSE    SLOT_MATERIAL_DIFFUSE0

#define SLOT_MATERIAL_NORMAL0    4
#define SLOT_MATERIAL_NORMAL1 	 5
#define SLOT_MATERIAL_NORMAL     SLOT_MATERIAL_NORMAL0

#define SLOT_MATERIAL_SPECULAR0  7
#define SLOT_MATERIAL_SPECULAR   SLOT_MATERIAL_SPECULAR0

/*
*	animation
*/
#define SLOT_ANIME_DEFORMED_MESH       0
#define SLOT_ANIME_BLEND_SHAPES        1


/*
*	terrain vt
*/
#define SLOT_TERRAIN_DEFORMED_MESH      0

#define SLOT_TERRAIN_VT_DIFFUSE  0
#define SLOT_TERRAIN_VT_NORMAL   1
#define SLOT_TERRAIN_VT_HEIGHT   2





/*
*	gbuffer bindings
*/
#define SLOT_GBUFFER_DIFFUSE      8
#define SLOT_GBUFFER_COMPACT      9
#define SLOT_GBUFFER_DEPTH        10
#define SLOT_GBUFFER_SPECULAR     11
#define SLOT_GBUFFER_MOTION       12
#define SLOT_GBUFFER_PREV_COMPACT 13


/*
*	env lighting, IBL
*/
#define SLOT_IBL_ENV           17
#define SLOT_IBL_LUT           18
#define SLOT_IBL_INTERGRATED   19
#define SLOT_IBL_IRRADIANCE    20


/*
*	 ssao
*/
#define SLOT_SSAO_NOISE        0


/*
*	 emissive
*/


/*
*	 resolve lighting
*/
#define SLOT_RESOLVE_REFLECTION   0
#define SLOT_RESOLVE_AO           1
#define SLOT_RESOLVE_LIGHTING     2
#define SLOT_RESOLVE_POST         3

/*
*	 hdr
*/


#define SLOT_DHR_INPUT1           0
#define SLOT_DHR_INPUT2           1
#define SLOT_DHR_POST             2



/*
*	 fsr
*/
#define SLOT_FSR_INPUT         0
#define SLOT_FSR_OUTPUT        0




/*
*	 raytracing
*/
#define SLOT_RT_SCENE              0
#define SLOT_RT_MATERIAL_DIFFUSE   2
#define SLOT_RT_MATERIAL_NORMAL    3
#define SLOT_RT_MATERIAL_SPECULAT  4

#define MATERIAL_RT_SLOT(slot)  (slot/3 + 2)

#define SLOT_RT_LIGHTING_LIGHTS     1

/*
*	 raytraced reflection
*/
#define SLOT_RT_REFLECTION_POST   	2
#define SLOT_RT_REFLECTION_ENV    	3
#define SLOT_RT_REFLECTION_TARGET 	0

/*
*	 raytraced diffuse lighting
*/
#define SLOT_RT_LIGHTING_TARGET    	0

/*
*		svgf denoising
*/
#define SLOT_SVGF_PREV_COLOR      0
#define SLOT_SVGF_PREV_MOMENT  	  1
#define SLOT_SVGF_INPUT           2

#define SLOT_SVGF_FILTER_COLOR    0
#define SLOT_SVGF_FILTER_MOMENT   1

/*
*	  light culling compute shader
*/
#define SLOT_LIGHT_CULLING_RESULT 0


#ifdef __cplusplus // c++ side 


#include "core/str.h"

/*
*   texture name to slot mapping
*/

typedef struct SlotName {
	String name;
	int slot;
}SlotName;


#define BEGIN_SLOT_MAPPING() static SlotName slotNames[] = {
#define END_SLOT_MAPPING() };
#define SLOT_MAPPING(name)  SlotName{ String(#name), SLOT_##name## }

// named texture to slot mappings
BEGIN_SLOT_MAPPING() 
	// materials
	SLOT_MAPPING(MATERIAL_DIFFUSE),
	SLOT_MAPPING(MATERIAL_NORMAL),
	SLOT_MAPPING(MATERIAL_SPECULAR),
	// ssao noise
	SLOT_MAPPING(SSAO_NOISE),
	// ibl 
	SLOT_MAPPING(IBL_ENV),
	SLOT_MAPPING(IBL_LUT),
	SLOT_MAPPING(IBL_INTERGRATED),
	SLOT_MAPPING(IBL_IRRADIANCE),
	// rt-reflection env map
	SLOT_MAPPING(RT_REFLECTION_ENV),
END_SLOT_MAPPING()


static int GetShaderBindingSlot(char* name) 
{
	for (auto i = 0; i < sizeof(slotNames)/sizeof(SlotName); ++i) {
		auto& slot = slotNames[i];
		if (slot.name == name) {
			return slot.slot;
		}
	}
	return -1;
}


#endif // __cplusplus


#endif // __SHADER_BINDING_MAP__
