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
private:
	// gameobject container
	List<GameObject> Objects;
public:
	Scene(Context * context);
	virtual ~Scene();
	// add to scene
	void AddGameObject(GameObject * obj);
	// update the scene. clear all dirty objects
	int Update(int ms);
};

#endif