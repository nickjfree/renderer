#include "GameObject.h"
#include "Scene\Scene.h"

USING_ALLOCATER(GameObject)

GameObject::GameObject(Context * context_) : Dirty(0), EventNode(context_)
{
}

GameObject::GameObject(Context * context_, String& Name) : Dirty(0), EventNode(context_)
{
	this->Name = Name;
}


GameObject::~GameObject() {
}



Matrix4x4& GameObject::GetTransform() {
	return GlobalTrans;
}

Vector3& GameObject::GetTranslation() {
	return Translate;
}
// get rotation as quaternion
Quaternion& GameObject::GetRotation() {
	return Rotation;
}

Component * GameObject::GetComponent(String& TypeName) {
	Vector<Component*>::Iterator Iter;
	Component * Comp = NULL;
	for (Iter = Components.Begin(); Iter != Components.End(); Iter++) {
		Comp = (*Iter);
		if (Comp->GetBaseTypeName() == TypeName) {
			return Comp;
		}
	}
	return NULL;
}


void GameObject::NotifyTransform() {
	Component * component = GetComponent(String("Renderer"));
	if (component) {
		component->OnTransform(this);
	}
	component = GetComponent(String("Light"));
	if (component) {
		component->OnTransform(this);
	}
	component = GetComponent(String("Camera"));
	if (component) {
		component->OnTransform(this);
	}
}

void GameObject::SetTranslation(Vector3& Translation_) {
	Translate = Translation_;
	Dirty = 1;
//	NotifyTransform();
}

void GameObject::SetRotation(Quaternion& Rotation_) {
	Rotation = Rotation_;
	Dirty = 1;
//	NotifyTransform();
}

void GameObject::SetTransform(Matrix4x4& Transform) {
	Dirty = 1;
}

void GameObject::Walk(float distance) {
	Translate = Translate + Look * distance;
	Dirty = 1;
}

void GameObject::Strife(float distance) {
	Translate = Translate + Right * distance;
	Dirty = 1;
}

// ASCEND
void GameObject::Ascend(float distance) {
	Translate = Translate + Up * distance;
	Dirty = 1;
}
// pitch
void GameObject::Pitch(float rad) {
	Quaternion DRotation = Quaternion();
	DRotation.RotationNormal(Right, rad);
	Rotation = Rotation * DRotation;
	Dirty = 1;
}
// yaw
void GameObject::Yaw(float rad) {
	Quaternion DRotation = Quaternion();
	DRotation.RotationNormal(Up, rad);
	Rotation = Rotation * DRotation;
	Dirty = 1;
}
// roll
void GameObject::Roll(float rad) {
	Quaternion DRotation = Quaternion();
	DRotation.RotationNormal(Look, rad);
	Rotation = Rotation * DRotation;
	Dirty = 1;
}

Component * GameObject::CreateComponent(String& type) {
	Component * comp = (Component*)context->CreateObject(type);
	return comp;
}

bool GameObject::AddComponent(Component * component) {
	if (GetComponent(component->GetTypeName())) {
		return false;
	}
	else {
		Components.PushBack(component);
		component->SetOwner(this);
		// call onattach
		component->OnAttach(this);
		return true;
	}
}

GameObject * GameObject::CreateGameObject(String& Name) {
	GameObject * SubObject = new GameObject(context, Name);
	SubObject->Sibling.InsertAfter(&Children);
	SubObject->Root = Root;
	SubObject->Parent = this;
	// insert to scene
	Root->AddGameObject(SubObject);
	return SubObject;
}

GameObject * GameObject::CreateGameObject(char * Name) {
	return CreateGameObject((String&)String(Name));
}

int GameObject::Subscribe(int Event, String& Callback) {
	// we are acctually subscribe script componet as a gameobject in scripting
	Component * comp = GetComponent(String("Script"));
	if (comp) {
		return comp->Subscribe(Event, Callback);
	}
	return 1;
}

int GameObject::SendEvent(int EventId) {
	Event * evt = Event::Create();
	evt->EventId = EventId;
	return context->BroadCast(evt);
}

int GameObject::Update(int Delta) {
	if (Dirty) {
		Up = Vector3(0, 1, 0) * Rotation;
		Right = Vector3(1, 0, 0) * Rotation;
		Look = Vector3(0, 0, 1) * Rotation;
		Up.Normalize();
		Look.Normalize();
		// notity component
		NotifyTransform();
		// clear dirty flag
		Dirty = 0;
	}
	return 0;
}