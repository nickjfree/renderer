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
	GameObject(Context * context_);
	GameObject(Context * context, String& Name);
	~GameObject();

	// get global transform of the gameobject
	Matrix4x4& GetTransform();
	// do we need a get local transform?
	// Matrix4x4&  GetLocalTransform();
	// get position in 3d space
	Vector3& GetTranslation();
	// get rotation as quaternion
	Quaternion& GetRotation();
	// Get componnen by component name hash
	Component * GetComponent(String& TypeName);

	//set translation
	void SetTranslation(Vector3& Translation);

	// set rotation
	void SetRotation(Quaternion& Rotation);
	// set transform
	void SetTransform(Matrix4x4& Transform);
	// add component
	bool AddComponent(Component * component);
	// create and add a component with template argument
	template <class T> 	T* CreateComponent();
	// create sub gameobject
	GameObject * CreateGameObject(String& Name);
	GameObject * CreateGameObject(char * Name);
	// Get parent
	GameObject * GetParent() {
		return Parent;
	}
	// Get Scene
	Scene * GetScene() {
		return Root;
	}
	// get name
	String& GetName() { return Name; }
	// set name
	void SetName(char * Name_) { Name = Name_; };
	void SetName(String& Name_) { Name = Name; };

protected:
	// name
	String Name;
	// translation 
	Vector3 Translate;
	// rotations as a quaternion
	Quaternion Rotation;
	// Local  matrix and Global  matrix transform
	Matrix4x4 LocalTrans;
	Matrix4x4 GlobalTrans;
	// component add to this gameobject
	Vector<Component *> Components;
	// parent object
	GameObject * Parent;
	// root object
	Scene * Root;
	// sub GameObjects
	LinkList<GameObject> Children;
	// sibling link to link children together
	LinkList<GameObject> Sibling;
	// listner, listening for the scenenode change event
	Vector<EventNode *> SceneListeners;
	// dirty flag, used for recursivelly update transform and rotation
	bool Diry;
	// nitoce transform
	void NotifyTransform();
};

#endif
