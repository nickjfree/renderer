#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "Scene\Component.h"
#include "Core\Allocater.h"
#include "Core\Shortcuts.h"
#include "Math\LinearMath.h"
#include "RenderLight.h"
/*
	light component
*/
class Light : public Component
{
	OBJECT(Light);
	BASEOBJECT(Light);
	DECLAR_ALLOCATER(Light);
private:
	RenderLight * renderLight;
	// light type
	int LightType;
	// radius
	float Rradius;
	// color
	Vector3 Color;
	// shadow flag
	int CastShadow;
public:
	enum {
		POINT,
		DIRECTION,
		SPOT
	};
public:
	Light(Context * context);
	virtual ~Light();
	// set redius
	void SetRadius(float Radius);
	// set shadow flag
	void SetShadowCast(int Flag);
	// set color
	void SetColor(Vector3& Color);
	// intensity
	void SetIntensity(float intensity);
	// on attach
	virtual int OnAttach(GameObject * GameObj);
	// Handler event
	virtual int HandleEvent(Event * Ev);
	// set model
	void SetModel(Model * model) { renderLight->SetModel(model); }
	// set material
	void SetMaterial(Material * material) { renderLight->SetMaterial(material); }
	// load
	virtual int Load(void * Raw, Level * level);
};

#endif
