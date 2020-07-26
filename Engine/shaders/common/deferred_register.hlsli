// common effect data
#ifndef  __DEFERRED_REGISTER__
#define  __DEFERRED_REGISTER__


Texture2D gDiffuseBuffer: register(t8);
Texture2D gCompactBuffer: register(t9);         // xy: normal  z: linearZ(half)  w: objectId,
Texture2D gDepthBuffer : register(t10);         // linearZ
Texture2D gSpecularBuffer: register(t11);
Texture2D gMotionVector: register(t12);         
Texture2D gPrevCompactBuffer: register(t13);

#endif