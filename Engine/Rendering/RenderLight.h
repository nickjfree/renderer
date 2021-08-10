#ifndef __RENDER_LIGHT__
#define __RENDER_LIGHT__

#include "RenderObject.h"
#include "RenderingCamera.h"


//typedef struct LightData
//{
//    // light color
//	Vector3 color;
//	// light direction
//	Vector3 direction;
//	// light position
//	Vector3 position;
//	// radius
//	float radius;
//	// intensity
//	float intensity;
//	// spotlight inner angle
//	float innerAngle; 
//	// spotlight outer angle
//	float outerAngle;
//	// rectlight width
//	float rectLightWidth;
//	// rectlight height
//	float rectLightHeight;
//	// light type
//	int type;
//	// pad
//	int pad;
//}LightData;




/*
	light node
*/
class RenderLight : public RenderObject
{
	DECLARE_ALLOCATER(RenderLight);
private:
	// light parameter
	// direction
	Vector3 Direction;
	// color
	Vector3 Color;
	// radius
	float Radius;
	// intensity
	float Intensity;
	// spot angle
	float SpotAngle = 0;
	// shadow cast
	int ShadowCast;
	// light camera
	RenderingCamera* LightCamera;
	// light type
	int LightType;
	// showmap id
	int ShadowMap = -1;
	// cblight
	CBLight cbLight;
public:
	enum {
		POINT,
		DIRECTION,
		SPOT,
		ENV,
		CAPSULE,
		RECT,
	};

public:
	RenderLight();
	~RenderLight();
	// compile
	int Compile(BatchCompiler* Compiler, int Stage, int Lod, Dict& StageParameter, RenderingCamera* Camera, RenderContext* Context);
	// qeueu cmdbuffer
	int Render(CommandBuffer* cmdBuffer, int stage, int lod, RenderingCamera* camera, RenderContext* renderContext);
	// update raytracing structure
	int UpdateRaytracingStructure(CommandBuffer* cmdBuffer, RenderingCamera* camera, RenderContext* renderContext);
	// set light type
	void SetLightType(int Type);
	// set radius
	void SetRadius(float r);
	// set color
	void SetColor(Vector3& Color);
	// set intensity
	void SetIntensity(float Intensity);
	// set direction
	void SetDirection(Vector3& Direction);
	// set shdowcast disable/enable
	void SetShadowCast(int Flag);
	// set shadow map
	void SetShadowMap(int id);
	// get radius
	float GetRadius() const { return Radius; }
	// get spot angle
	float GetSpotAngle() const { return SpotAngle; }
	// get color
	const Vector3& GetColor() const { return Color; }
	// get direction
	const Vector3& GetDirection() const { return Direction; }
	// get intensity
	float GetIntensity() const { return Intensity; }
	// get type
	int GetLightType() const { return LightType; }
	// get shadow casting
	int GetShadowCasting() const { return ShadowCast; }
	// get Light Camera
	RenderingCamera* GetLightCamera();
	// get lightdata
	LightData GetLightData();
	// get light info
	Vector3 GetDesc();
};

#endif