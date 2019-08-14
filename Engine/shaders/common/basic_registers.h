#ifndef __BASIC_REGISTERS__
#define __BASIC_REGISTERS__

/*
    basic register setup
*/

// samplers
SamplerState gSam : register(s0);
SamplerState gSamBilinear :register(s1);
SamplerState gSamPoint :register(s2);

//multi texture, 8 texture at most 
Texture2D gDiffuseMap0 : register(t0);
Texture2D gDiffuseMap1 : register(t1);
Texture2D gDiffuseMap2 : register(t2);
Texture2D gDiffuseMap3 : register(t3);
Texture2D gNormalMap0 : register(t4);
Texture2D gNormalMap1 : register(t5);
Texture2D gNormalMap2 : register(t6);
Texture2D gNormalMap3 : register(t7);
Texture2D gSpecularMap0 : register(t8);

// deferred shading gbiffer
Texture2D gDepthBuffer : register(t9);
Texture2D gNormalBuffer: register(t10);
Texture2D gDiffuseBuffer: register(t11);
Texture2D gSpecularBuffer: register(t12);
Texture2D gLightBuffer : register(t13);


#endif