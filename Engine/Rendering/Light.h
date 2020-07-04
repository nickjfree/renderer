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
	DECLARE_ALLOCATER(Light);
private:
	RenderLight* renderLight;
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
		SPOT,
		ENV,
	};
public:
	Light(Context* context);
	virtual ~Light();
	// set type
	void SetType(int Type) { renderLight->SetLightType(Type); };
	// set redius
	void SetRadius(float Radius);
	// set shadow flag
	void SetShadowCast(int Flag);
	// set color
	void SetColor(Vector3& Color);
	// intensity
	void SetIntensity(float intensity);
	// direction
	void SetDirection(Vector3& Direction);
	// on attach
	virtual int OnAttach(GameObject* GameObj);
	// Handler event
	virtual int HandleEvent(Event* Ev);
	// on transform
	// transform change
	virtual int OnTransform(GameObject* GameObj);
	// set model
	void SetModel(Model* model) { renderLight->SetModel(model); }
	// set material
	void SetMaterial(Material* material) { renderLight->SetMaterial(material); }
	// load
	virtual int Load(void* Raw, Level* level);
	// save
	virtual int Save(Serializer* levelFile, Level* level);
};

#endif
