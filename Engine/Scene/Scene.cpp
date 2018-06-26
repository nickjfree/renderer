#include "Scene.h"

USING_ALLOCATER(Scene);

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
		if (!obj->Destroyed) {
			obj->Update(ms);
		} else {
			Destroyed.PushBack(obj);
		}
	}
	// handle destroyed objs
	int Size = Destroyed.Size();
	for (int i = 0; i < Size; i++) {
		GameObject * obj = Destroyed[i];
		Objects.Remove(obj);
		obj->DecRef();
	}
	Destroyed.Empty();
	return 0;
}
