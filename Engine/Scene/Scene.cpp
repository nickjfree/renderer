#include "Scene.h"


Scene::Scene(Context * context) : GameObject(context)
{
	Root = this;
}


Scene::~Scene()
{
}
