#include "Component.h"
#include "GameObject.h"

USING_ALLOCATER(Component)

Component::Component(Context * context_):EventNode(context_),Destroyed(0)
{
}


Component::~Component()
{
}

void Component::SetOwner(GameObject * Owner) {
	this->Owner = Owner;
}

GameObject * Component::GetOwner() {
	return Owner;
}

int Component::BroadCast(Event * Evt) {
	if (Owner) {
		return SendEvent(Owner, Evt);
	}
	return -1;
}

int Component::OnAttach(GameObject * GameObj) {
	return 0;
}

int Component::Load(void * Raw, Level * level) {
	return 0;
}

int Component::OnTransform(GameObject * GameObj) {
	return 0;
}

int Component::OnDestroy(GameObject * GameObj) {
	DisableEvent();
	Destroyed = 1;
	return 0;
}
	
