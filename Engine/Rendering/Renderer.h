#ifndef __RENDERER__
#define __RENDERER__

#include "Scene\Component.h"
/*
	renderer component
*/
class Renderer : public Component
{
	OBJECT(Renderer);
	DECLARE_ALLOCATER(Renderer);
	//	UUID();
public:
	Renderer(Context* context);
	virtual ~Renderer();
	virtual int OnTransform(GameObject* GameObj);
};

#endif