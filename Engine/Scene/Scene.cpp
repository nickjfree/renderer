#include "Scene.h"

USING_ALLOCATER(Scene);


// gameobject name lookup
HashMap<String, GameObject*, 1024> Scene::gameObjectLookUp_;


Scene::Scene(Context* context) : GameObject(context) {
	Root = this;
}


Scene::~Scene() {
}

void Scene::AddGameObject(GameObject* obj) 
{
	Objects.Insert(obj);
	// update lookup table
	gameObjectLookUp_[obj->GetName()] = obj;
}

int Scene::Update(int ms) 
{
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
		// decref is assumed to delete this gameobject if no script referencing it
		obj->DecRef();
	}
	Destroyed.Reset();
	return 0;
}

bool Scene::IsEmpty() 
{
	return Objects.Empty();
}

// find gameobject by name
GameObject* Scene::GetGameObject(const String& name)
{
	auto iter = gameObjectLookUp_.Find(name);
	if (iter == gameObjectLookUp_.End()) {
		return nullptr;
	} else  {
		return *iter;
	}
}