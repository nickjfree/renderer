#ifndef __EXPORT_H__
#define __EXPORT_H__

#include "Proxy.h"

#include "Scene\GameObject.h"
#include "Scene\Scene.h"
#include "Rendering\Model.h"
#include "Rendering\MeshRenderer.h"

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
	METHOD(AddComponent, &GameObject::AddComponent)
END_PROXY()

// Model
BEGIN_PROXY(Model)

END_PROXY()

// Renderer
BEGIN_PROXY(MeshRenderer)
	METHOD(SetMaterial, &MeshRenderer::SetMaterial)
	METHOD(SetModel, &MeshRenderer::SetModel)
END_PROXY()

//Matrtial
BEGIN_PROXY(Material)
	METHOD(GetUrl, &Material::GetUrl)
END_PROXY()

#endif 
