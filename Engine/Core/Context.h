#ifndef __GAME_CONTEXT__
#define __GAME_CONTEXT__


#include "Container\HashMap.h"
#include "Core\Str.h"
//#include "Core\ObjectFactory.h"


/*
	Engine context. can register subsystem. and event handlers
*/

class EventNode;
class Event;
class System;
class ObjectFactory;
class Object;

class Context
{
private:
	// Global  Event Bus
	EventNode * GlobalEventNode;
	// Subsystem
	HashMap<String, System *> Subsystems;
	// objects and typenames
	HashMap<String, ObjectFactory*> ObjectFactories;
public:
	Context();
	virtual ~Context();
	System * RegisterSubsystem(String& Name, System * Subsystem);
	System * GetSubsystem(String& Name);

	ObjectFactory * RegisterObject(String& Name, ObjectFactory * Factory);
	Object * CreateObject(String& Name);

	// SendEvent 
	int SendEvent(EventNode * Receiver, Event * event);
	// BroadCast
	int BroadCast(Event * event);
	// Add eventhandler for system
	int SubscribeFor(System * subsystem, int EventId);
	// remove eventhandler
	int UnSubscribeFor(System * subsystem, int EventId);


	// subsystem
	template <class T> T*  RegisterSubsystem() {
		T * Subsystem = new T(this);
		Subsystem->Initialize();
		RegisterSubsystem(T::GetTypeNameStatic(), Subsystem);
		return Subsystem;
	}
	//get subsystem
	template <class T> T* GetSubsystem() {
		return (T*)GetSubsystem(T::GetTypeNameStatic());
	}

	//register object
	template <class T> void RegisterObject() {
		ObjectFactory * Factory = new ObjectFactoryImp<T>();
		RegisterObject(T::GetBaseTypeNameStatic(), Factory);
	}

	// createobject 
	template <class T> T* CreateObject() {
		return (T*)CreateObject(T::GetTypeNameStatic());
	}
};

#endif