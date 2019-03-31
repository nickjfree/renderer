#include "Context.h"
#include "EventNode.h"
#include "Core\System.h"
#include "Core\ObjectFactory.h"


Context::Context()
{
	GlobalEventNode = new EventNode(this);
}


Context::~Context()
{
}


int Context::SubscribeFor(EventNode * node, int EventId) {
	return node->SubscribeTo(GlobalEventNode, EventId);
}


int Context::UnSubscribeFor(EventNode * node, int EventId) {
	return node->UnSubscribe(GlobalEventNode, EventId);
}


int Context::SendEvent(EventNode * Receiver, Event * event){
	return Receiver->HandleEvent(event);
}

int Context::BroadCast(Event * event) {
	GlobalEventNode->HandleEvent(event);
	return 0;
}



System * Context::RegisterSubsystem(const String& Name, System * Subsystem) {
	Subsystems[const_cast<String&>(Name)] = Subsystem;
	return Subsystem;
}
System * Context::GetSubsystem(const String& Name) {
	HashMap<String, System *>::Iterator Iter;
	Iter = Subsystems.Find(const_cast<String&>(Name));
	if (Iter != Subsystems.End()) {
		return *Iter;
	}
	return NULL;
}

ObjectFactory * Context::RegisterObject(const String& Name, ObjectFactory * Factory) {
	ObjectFactories[const_cast<String&>(Name)] = Factory;
	return Factory;
}

Object * Context::CreateObject(const String& Name) {
	ObjectFactory * Factory = 0;
	Object * Obj;
	HashMap<String, ObjectFactory*>::Iterator Iter;
	Iter = ObjectFactories.Find(const_cast<String&>(Name));
	if (Iter != ObjectFactories.End()) {
		Factory = *Iter;
		Obj = (Object*)Factory->CreateObject(this);
		return Obj;
	}
	return NULL;
}
