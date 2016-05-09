#ifndef __SCENE__
#define __SCENE__

#include "GameObject.h"

/*
	Scene. Also the root of all gameobjects in a scene
*/

class Scene : public GameObject
{
	OBJECT(Scene);
	BASEOBJECT(GameObject);
public:
	Scene(Context * context);
	virtual ~Scene();
};

#endif