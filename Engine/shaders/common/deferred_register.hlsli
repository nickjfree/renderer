// common effect data
#ifndef  __DEFERRED_REGISTER__
#define  __DEFERRED_REGISTER__


Texture2D gDepthBuffer : register(t9);
Texture2D gNormalBuffer: register(t10);
Texture2D gDiffuseBuffer: register(t11);
Texture2D gSpecularBuffer: register(t12);
Texture2D gMotionVector: register(t13);


#endif