#ifndef __EXPORT_H__
#define __EXPORT_H__

#include "Proxy.h"

#include "Scene\GameObject.h"
#include "Scene\Scene.h"
#include "Rendering\Model.h"
#include "Rendering\MeshRenderer.h"
#include "Input\InputSystem.h"
#include "Physics\PhysicsObject.h"
#include "Animation\Animator.h"


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
METHOD(ListModels, &Level::ListModels)
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
METHOD(CreateGameObject, (GameObject * (GameObject::*)(char *))&GameObject::CreateGameObject)
METHOD(AddComponent, &GameObject::AddComponent)
METHOD(Walk, &GameObject::Walk)
METHOD(Strife, &GameObject::Strife)
METHOD(Ascend, &GameObject::Ascend)
METHOD(Pitch, &GameObject::Pitch)
METHOD(Yaw, &GameObject::Yaw)
METHOD(Roll, &GameObject::Roll)
METHOD(Destroy, &GameObject::Destroy)
METHOD(GetRef, &GameObject::GetRef)
METHOD(GetObjectId, &GameObject::GetObjectId)
METHOD(IsDestroyed, &GameObject::IsDestroyed)
METHOD(GetComponent, &GameObject::GetComponent)
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

// Input
BEGIN_PROXY(InputSystem)
METHOD(GetAction, &InputSystem::GetAction)
END_PROXY()

// Physics
BEGIN_PROXY(PhysicsObject)
METHOD(CreateShapeFromModel, &PhysicsObject::CreateShapeFromModel)
METHOD(SetObjectType, (void (PhysicsObject::*)(int))&PhysicsObject::SetObjectType)
END_PROXY()

// Animation
BEGIN_PROXY(BlendingNode)
METHOD(SetParameter, &BlendingNode::SetParameter)
END_PROXY()

BEGIN_PROXY(Animator)
METHOD(GetBlendingNode, &Animator::GetBlendingNode)
END_PROXY()

#endif 
