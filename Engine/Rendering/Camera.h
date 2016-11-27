#ifndef __CAMERA__
#define __CAMERA__


#include "Scene\Component.h"
#include "Core\Allocater.h"
#include "Core\Shortcuts.h"

#include "Rendering\RenderingCamera.h"
/*
	camera component
*/
class Camera : public Component
{
	BASEOBJECT(Camera);
	OBJECT(Camera);
	DECLAR_ALLOCATER(Camera);
	UUID()
private:
	RenderingCamera * RenderCamera;
protected:
	int DHR;
	int Occlusion;
public:
	enum  RenderPath {
		FORWARD,
		DEFERRED,
		LIGHT_PRE,
	};
public:
	Camera(Context * context);
	virtual ~Camera();

	virtual int HandleEvent(Event * Ev);
	// on transform
	virtual int OnTransform(GameObject * GameObj);
	// setters
	void Enable();
	void Disable();
};

#endif


