#include "GameObject.h"

USING_ALLOCATER(GameObject)

GameObject::GameObject(Context * context_) : EventNode(context_)
{
}

GameObject::GameObject(Context * context_, String& Name) : EventNode(context_)
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
	NotifyTransform();
}

void GameObject::SetRotation(Quaternion& Rotation_) {
	Rotation = Rotation_;
	NotifyTransform();
}

void GameObject::SetTransform(Matrix4x4& Transform) {
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