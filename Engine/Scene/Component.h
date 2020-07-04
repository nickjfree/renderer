#ifndef __COMPONENT__
#define __COMPONENT__

#include "../Core/Allocater.h"
#include "../Core/EventNode.h"
#include "../Core/Shortcuts.h"
#include "Resource\Level.h"
#include "Resource\LevelDesc.h"

//#include "../Core/Allocater.h"

class GameObject;
// basic componont class
class Component : public EventNode
{

	DECLARE_ALLOCATER(Component);
	OBJECT(Component);
	BASEOBJECT(Component);
protected:
	GameObject* Owner;
public:
	Component(Context* context_);
	virtual ~Component();
	unsigned int GetStringHash();
	// Get set gameobject
	void SetOwner(GameObject* Owner);
	GameObject* GetOwner();
	// broadcast event to parent gameobject
	int BroadCast(Event* Evt);
	// on add to gameobject
	virtual int OnAttach(GameObject* GameObj);
	// on transform
	virtual int OnTransform(GameObject* GameObj);
	// on gameobejct destory
	virtual int OnDestroy(GameObject* GameObj);
	// load from raw filedata and level interface
	virtual int Load(void* Raw, Level* level);
	// save to raw data
	virtual int Save(Serializer* levelFile, Level* level);
};

#endif
