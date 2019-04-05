#ifndef __RENDER_LIGHT__
#define __RENDER_LIGHT__

#include "RenderObject.h"
#include "RenderingCamera.h"
/*
	light node
*/
class RenderLight : public RenderObject
{
	DECLAR_ALLOCATER(RenderLight);
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
	float SpotAngle;
	// shadow cast
	int ShadowCast;
	// light camera
	RenderingCamera * LightCamera;
	int LightType;
	enum {
		POINT,
		DIRECTION,
		SPOT,
		ENV,
	};
private:
	void UpdateLightView();

public:
	RenderLight();
	~RenderLight();
	 // compile
	int Compile(BatchCompiler * Compiler, int Stage, int Lod, Dict& StageParameter, RenderingCamera * Camera, RenderContext * Context);
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
	// get shadow casting
	int GetShadowCasting() const { return ShadowCast; }
	// get Light Camera
	RenderingCamera * GetLightCamera();
};

#endif