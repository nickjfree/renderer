#ifndef __BASIC_REGISTERS__
#define __BASIC_REGISTERS__

/*
    basic register setup
*/

//multi texture, 8 texture at most 
Texture2D gDiffuseMap0 : register(t0);

Texture2D gNormalMap0 : register(t4);

Texture2D gSpecularMap0 : register(t8);

// // deferred shading gbuffer
// Texture2D gDepthBuffer : register(t9);
// Texture2D gNormalBuffer: register(t10);
// Texture2D gDiffuseBuffer: register(t11);
// Texture2D gSpecularBuffer: register(t12);
// Texture2D gMotionVector: register(t13);

#endif