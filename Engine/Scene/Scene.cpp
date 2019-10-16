#include "Scene.h"

USING_ALLOCATER(Scene);

Scene::Scene(Context* context) : GameObject(context) {
	Root = this;
}


Scene::~Scene() {
}

void Scene::AddGameObject(GameObject* obj) {
	Objects.Insert(obj);
}

int Scene::Update(int ms) {
	for (auto Iter = Objects.Begin(); Iter != Objects.End(); Iter++) {
		GameObject* obj = *Iter;
		if (!obj->Destroyed) {
			obj->Update(ms);
		}
		else {
			Destroyed.PushBack(obj);
		}
	}
	// handle destroyed objs
	int Size = Destroyed.Size();
	for (int i = 0; i < Size; i++) {
		GameObject* obj = Destroyed[i];
		Objects.Remove(obj);
		// decref is assumed to delete this gameobject
		obj->DecRef();
	}
	Destroyed.Empty();
	return 0;
}


bool Scene::IsEmpty() {
	return Objects.Empty();
}