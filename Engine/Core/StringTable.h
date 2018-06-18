#ifndef __STR_TAB__
#define __STR_TAB__

#include "Core\Str.h"


#define MAKE_HASH_STRING(name) String hash_string::##name = String(#name);

class hash_string {
	// strings
public:

	// renderer
	static String gWorldViewProjection;
	static String InstanceWVP;
	static String InstanceWV;
	static String gWorldViewMatrix;
	static String gInvertViewMaxtrix;
	static String gProjectionMatrix;
	static String gViewPoint;
	static String gLightPosition;
	static String gLightDirection;
	static String gLightViewProjection;
	static String gShadowMap;
	static String gRadiusIntensity;
	static String gLightColor;
	static String gScreenSize;
	static String gSampleOffsets;
	static String gPostBuffer;
	static String gDiffuseMap1;
	static String gDiffuseMap0;
	static String gSampleWeights;
	static String gTimeElapse;
	static String gSpecular;
	static String gSkinMatrix;
	// event string
	static String Level;
	static String RenderObject;
	static String model;
};

#endif