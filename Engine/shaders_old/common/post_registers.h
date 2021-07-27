// common effect data
#ifndef  __POST_REGISTERS__
#define  __POST_REGISTERS__


// hdr 
Texture2D gDiffuseMap1 : register(t1);

// pingpong buffers
Texture2D gPostBuffer : register(t14); 
Texture2D gFinalBuffer: register(t15);

#endif