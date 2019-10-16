#ifndef __GAME_OBJ__
#define __GAME_OBJ__

#include "Component.h"
#include "Math\LinearMath.h"
#include "../Container/Vector.h"
#include "../Container/LinkList.h"


/*
   Entity class of the Entity/Component System.
   The GameObject works just like scenenode in a scenegragh.
   There is a different between scenegragh and  quadtree, octree, bsp.
   This is the traditional scenegragh alse a container of components
*/

class Scene;

class GameObject : public EventNode
{
	friend GameObject;
	OBJECT(GameObject);
	BASEOBJECT(GameObject);
	DECLAR_ALLOCATER(GameObject)
public:
	GameObject(Context* context_);
	GameObject(Context* context, const String& Name);
	~GameObject();

	// get global transform of the gameobject
	Matrix4x4& GetTransform();
	// do we need a get local transform?
	// Matrix4x4&  GetLocalTransform();
	// get position in 3d space
	Vector3& GetTranslation();
	// get rotation as quaternion
	Quaternion& GetRotation();
	// global position
	Vector3& GetWorldTranslation();
	// global rotation
	Quaternion& GetWorldRotation();

	// Get componnen by component name hash
	Component* GetComponent(const String& TypeName);
	// Get Look
	Vector3& GetLook() { return Look; }
	// Get Up
	Vector3& GetUp() { return Up; }
	// Get Right
	Vector3& GetRight() { return Right; }
	//set translation
	void SetTranslation(Vector3& Translation);
	// set rotation
	void SetRotation(Quaternion& Rotation);
	// set transform
	void SetTransform(Matrix4x4& Transform);
	// move functions
	void Walk(float Distance);
	// strife left
	void Strife(float Distance);
	// ASCEND
	void Ascend(float Distance);
	// pitch
	void Pitch(float Rad);
	// yaw
	void Yaw(float Rad);
	// roll
	void Roll(float Rad);
	// add component
	bool AddComponent(Component* component);
	// destory
	void Destroy();
	// create and add a component with template argument
	template <class T> 	T* CreateComponent();
	// create component, but not add it to the gameobject
	Component* CreateComponent(const String& type);
	// create sub gameobject
	GameObject* CreateGameObject(const String& Name);
	// attach subobject
	void Attach(GameObject* Sub);
	// Get parent
	GameObject* GetParent() {
		return Parent;
	}
	// Get Scene
	Scene* GetScene() {
		return Root;
	}
	// get name
	const String& GetName() const { return Name; }
	// set name
	void SetName(const String& Name_) { Name = Name; };
	// subscript event for scripts
	int Subscribe(int Event, String& Callback);
	// send event
	int SendEvent(int Event);
	// update
	int Update(int Delta);
	//  save
	int Save(Serializer* levelFile, Level* level);
protected:
	// name
	String Name;
	// translation 
	Vector3 Translate;
	// rotations as a quaternion
	Quaternion Rotation;
	// Local  matrix and Global  matrix transform
	Matrix4x4 LocalTrans;
	// global vectors
	Vector3 GlobalTranslate;
	// global rotation
	Quaternion GlobalRotation;
	//  global transform
	Matrix4x4 GlobalTrans;
	// vectors
	Vector3 Up, Look, Right;
	// component add to this gameobject
	Vector<Component*> Components;
	// parent object
	GameObject* Parent;
	// root object
	Scene* Root;
	// sub GameObjects
	LinkList<GameObject> Children;
	// sibling link to link children together
	LinkList<GameObject> Sibling;
	// dirty flag, used for recursivelly update transform and rotation
	bool Dirty;
protected:
	// nitoce transform
	void NotifyTransform();
	// make dirty
	void MakeDirty();
	// make clean
	void MakeClean();
};

#endif
