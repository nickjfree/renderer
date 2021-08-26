#include "GameObject.h"
#include "Scene\Scene.h"

USING_ALLOCATER(GameObject)

GameObject::GameObject(Context* context_) : Dirty(0), EventNode(context_), Parent(nullptr), Root(nullptr) {
	Sibling.Owner = this;
	Children.Owner = this;
}

GameObject::GameObject(Context* context_, const String& Name) : Dirty(0), EventNode(context_) {
	this->Name = Name;
	Sibling.Owner = this;
	Children.Owner = this;
}


GameObject::~GameObject() {
	//printf_s("destruct game object %d \n", ObjectId);
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

Vector3& GameObject::GetWorldTranslation() {
	MakeClean();
	return GlobalTranslate;
}
// get rotation as quaternion
Quaternion& GameObject::GetWorldRotation() {
	MakeClean();
	return GlobalRotation;
}

Component* GameObject::GetComponent(const String& TypeName) {
	Component* Comp = NULL;
	for (auto Iter = Components.Begin(); Iter != Components.End(); Iter++) {
		Comp = (*Iter);
		if (Comp->GetBaseTypeName() == TypeName) {
			return Comp;
		}
	}
	return NULL;
}

void GameObject::NotifyTransform() {
	Component* component = GetComponent("Renderer");
	if (component) {
		component->OnTransform(this);
	}
	component = GetComponent("Light");
	if (component) {
		component->OnTransform(this);
	}
	component = GetComponent("Camera");
	if (component) {
		component->OnTransform(this);
	}
	component = GetComponent("PhysicsObject");
	if (component) {
		component->OnTransform(this);
	}
	component = GetComponent("Volume");
	if (component) {
		component->OnTransform(this);
	}
}

void GameObject::SetTranslation(Vector3& Translation_) {
	Translate = Translation_;
	MakeDirty();
	//	NotifyTransform();
}

void GameObject::SetRotation(Quaternion& Rotation_) {
	Rotation = Rotation_;
	MakeDirty();
	//	NotifyTransform();
}

void GameObject::SetTransform(Matrix4x4& Transform) {
	Rotation.FromMatrix(Transform);
	Translate = Vector3(0, 0, 0) * Transform;
	MakeDirty();
}

void GameObject::Walk(float distance) {
	Translate = Translate + Look * distance;
	MakeDirty();
}

void GameObject::Strife(float distance) {
	Translate = Translate + Right * distance;
	MakeDirty();
}

// ASCEND
void GameObject::Ascend(float distance) {
	Translate = Translate + Up * distance;
	MakeDirty();
}
// pitch
void GameObject::Pitch(float rad) {
	Quaternion DRotation = Quaternion();
	DRotation.RotationNormal(Right, rad);
	Rotation = Rotation * DRotation;
	MakeDirty();
}
// yaw
void GameObject::Yaw(float rad) {
	Quaternion DRotation = Quaternion();
	DRotation.RotationNormal(Up, rad);
	Rotation = Rotation * DRotation;
	MakeDirty();
}
// roll
void GameObject::Roll(float rad) {
	Quaternion DRotation = Quaternion();
	DRotation.RotationNormal(Look, rad);
	Rotation = Rotation * DRotation;
	MakeDirty();
}

Component* GameObject::CreateComponent(const String& type) {
	Component* comp = (Component*)context->CreateObject(type);
	return comp;
}

Object* GameObject::CreateObject(const String& type) {
	return context->CreateObject(type);
}

bool GameObject::AddComponent(Component* component) {
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

GameObject* GameObject::CreateGameObject(const String& Name) {
	GameObject* SubObject = new GameObject(context, Name);
	SubObject->Sibling.InsertAfter(&Children);
	SubObject->Root = Root;
	SubObject->Parent = this;
	// make sub obejct dirty, so its transfom will inherent the current partent
	SubObject->MakeDirty();
	// insert to scene
	Root->AddGameObject(SubObject);
	return SubObject;
}

void GameObject::Destroy() {
	if (Destroyed) {
		return;
	}
	for (auto Iter = Components.Begin(); Iter != Components.End(); Iter++) {
		auto Comp = (*Iter);
		Comp->OnDestroy(this);
		Comp->DecRef();
	}
	// give up children to parent
	Parent;

	LinkList<GameObject>::Iterator ChildrnnIter;
	while (Children.Begin() != Children.End()) {
		ChildrnnIter = Children.Begin();
		GameObject* child = *ChildrnnIter;
		Parent->Attach(child);
	}
	// remove from parent
	Sibling.Remove();
	Destroyed = true;
}

void GameObject::Attach(GameObject* Sub) {
	MakeClean();
	Sub->MakeClean();
	Matrix4x4 Inv;
	Matrix4x4 Rel;
	Matrix4x4::Inverse(GlobalTrans, &Inv);
	// get relative trans from this to sub
	Rel = Sub->GetTransform() * Inv;
	Sub->Translate = Vector3(0, 0, 0) * Rel;
	Sub->Rotation.FromMatrix(Rel);
	// switch parent
	if (Sub->Parent) {
		Sub->Sibling.Remove();
	}
	Sub->Parent = this;
	Sub->Sibling.InsertAfter(&Children);
	// set dirty
	Sub->MakeDirty();
}

int GameObject::Subscribe(int Event, String& Callback) {
	// we are acctually subscribe script componet as a gameobject in scripting
	Component* comp = GetComponent("Script");
	if (comp) {
		return comp->Subscribe(Event, Callback);
	}
	return 1;
}

int GameObject::SendEvent(int EventId) {
	Event* evt = Event::Create();
	evt->EventId = EventId;
	return context->BroadCast(evt);
}

int GameObject::Update(int Delta) {
	MakeClean();
	return 0;
}

void GameObject::MakeDirty() {
	if (!Dirty) {
		Dirty = 1;
		Root;
		// make all children dirty
		LinkList<GameObject>::Iterator Iter;
		for (Iter = Children.Begin(); Iter != Children.End(); Iter++) {
			GameObject* child = *Iter;
			child->MakeDirty();
		}
	}
}

void GameObject::MakeClean() {
	if (Dirty) {
		Up = Vector3(0, 1, 0) * Rotation;
		Right = Vector3(1, 0, 0) * Rotation;
		Look = Vector3(0, 0, 1) * Rotation;
		Up.Normalize();
		Look.Normalize();
		Right.Normalize();
		// make world tranform
		Parent->MakeClean();
		LocalTrans.Tranform(Translate, Rotation);
		GlobalTrans = LocalTrans * Parent->GlobalTrans;
		GlobalTranslate = Vector3(0, 0, 0) * GlobalTrans;
		GlobalRotation.FromMatrix(GlobalTrans);
		// clear dirty flag
		Dirty = 0;
		// notity component
		NotifyTransform();
	}
}

int GameObject::Save(Serializer* levelFile, Level* level) {
	ObjectEntry Entry{};
	strcpy_s(Entry.Name, Name.ToStr());
	Entry.Position = GlobalTranslate;
	Entry.Rotation = GlobalRotation;
	Entry.Scale = Vector3(1, 1, 1);
	Entry.NumComponents = 0;

	for (auto iter = Components.Begin(); iter != Components.End(); iter++) {
		auto component = *iter;
		if (component->GetBaseTypeName() == "Renderer" || component->GetBaseTypeName() == "Light") {
			Entry.NumComponents++;
		}
	}
	// write gameobject entry
	levelFile->Write(&Entry, sizeof(Entry));

	for (auto iter = Components.Begin(); iter != Components.End(); iter++) {
		auto component = *iter;
		component->Save(levelFile, level);
	}
	return 0;
}