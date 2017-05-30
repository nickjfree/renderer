#ifndef __EXPORT_H__
#define __EXPORT_H__

#include "Proxy.h"

#include "Scene\GameObject.h"
#include "Scene\Scene.h"

// proxy Scene
BEGIN_PROXY(Scene)
	METHOD(CreateGameObject, (GameObject * (Scene::*)(char *))&Scene::CreateGameObject)
	METHOD(CreateComponent, &Scene::CreateComponent)
END_PROXY()

// proxy Level
BEGIN_PROXY(Level)
	METHOD(GetModel, &Level::GetModel)
	METHOD(GetMaterial, &Level::GetMaterial)
	METHOD(GetScene, &Level::GetScene)
END_PROXY()

// proxy GameObject
BEGIN_PROXY(GameObject)
	METHOD(GetName, &GameObject::GetName)
	METHOD(SetName, (void (GameObject::*)(char *))&GameObject::SetName)
	METHOD(SetTranslation, &GameObject::SetTranslation)
	METHOD(GetTranslation, &GameObject::GetTranslation)
	METHOD(SetRotation, &GameObject::SetRotation)
	METHOD(GetRotation, &GameObject::GetRotation)
	METHOD(Subscribe, &GameObject::Subscribe)
	METHOD(SendEvent, &GameObject::SendEvent)
END_PROXY()


#endif 
