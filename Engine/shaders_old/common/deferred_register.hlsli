// common effect data
#ifndef  __DEFERRED_REGISTER__
#define  __DEFERRED_REGISTER__


Texture2D gDiffuseBuffer: register(t8);         // 4 bytes
Texture2D gCompactBuffer: register(t9);         // 8 bytes  xy: normal  z: linearZ(half)  w: objectId,
Texture2D gDepthBuffer : register(t10);         // 4 bytes linearZ
Texture2D gSpecularBuffer: register(t11);       // 4 bytes
Texture2D gMotionVector: register(t12);         // 8 bytes
Texture2D gPrevCompactBuffer: register(t13);    // 8 bytes

#endif