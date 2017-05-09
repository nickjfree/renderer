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
	return GlobalEventNode->AddEventHandler(EventId, node);
}


int Context::UnSubscribeFor(EventNode * node, int EventId) {
	return GlobalEventNode->RemoveEventHandler(EventId, node);
}


int Context::SendEvent(EventNode * Receiver, Event * event){
	return Receiver->HandleEvent(event);
}

int Context::BroadCast(Event * event) {
	GlobalEventNode->HandleEvent(event);
	return 0;
}



System * Context::RegisterSubsystem(String& Name, System * Subsystem) {
	Subsystems[Name] = Subsystem;
	return Subsystem;
}
System * Context::GetSubsystem(String& Name) {
	HashMap<String, System *>::Iterator Iter;
	Iter = Subsystems.Find(Name);
	if (Iter != Subsystems.End()) {
		return *Iter;
	}
	return NULL;
}

ObjectFactory * Context::RegisterObject(String& Name, ObjectFactory * Factory) {
	ObjectFactories[Name] = Factory;
	return Factory;
}

Object * Context::CreateObject(String& Name) {
	ObjectFactory * Factory = 0;
	Object * Obj;
	HashMap<String, ObjectFactory*>::Iterator Iter;
	Iter = ObjectFactories.Find(Name);
	if (Iter != ObjectFactories.End()) {
		Factory = *Iter;
		Obj = (Object*)Factory->CreateObject(this);
		return Obj;
	}
	return NULL;
}
