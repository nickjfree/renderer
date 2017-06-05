#include "Scene.h"


Scene::Scene(Context * context) : GameObject(context) {
	Root = this;
}


Scene::~Scene() {
}

void Scene::AddGameObject(GameObject * obj) {
	Objects.Insert(obj);
}

int Scene::Update(int ms) {
	List<GameObject>::Iterator Iter;
	for (Iter = Objects.Begin(); Iter != Objects.End(); Iter++) {
		GameObject * obj = *Iter;
		obj->Update(ms);
	}
	return 0;
}
