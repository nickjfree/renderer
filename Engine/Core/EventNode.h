
#ifndef __EVENT_Node__
#define __EVENT_Node__

#include "Shortcuts.h"
#include "Object.h"
#include "Variant.h"
#include "Event.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include "../Container/HashMap.h"
#include "../Container/List.h"



using std::vector;
using std::unordered_map;
using std::shared_ptr;
using std::weak_ptr;

/* 
basic event processing node, can handle events and send events.
*/
class EventNode : public Object
{
private:
	// a map from certain event id to Event handlers, EventBus
	HashMap<int, List<EventNode>> EventChannel;
	//unordered_map<int, vector<shared_ptr<EventNode>>> EventChannel;
	// for test, 
	shared_ptr<Object> Ref;

public:
	EventNode(Context * context_);
	virtual ~EventNode();
	// register self to other eventnode, and listen for event
	int SubscribeTo(EventNode * Hub, int EventId);
	// register self to global eventnode in context
	virtual int Subscribe(int EventId, String& Callback) { return 1; };
	// unregister
	int UnSubscribe(EventNode * Hub, int EventId);
	// add event handler
	int AddEventHandler(int EventId, EventNode* Handler);
	// remove event handler
	int RemoveEventHandler(int EventId, EventNode* Handler);
	// send event to eventnode
	int SendEvent(EventNode * Receiver, Event * Evt);
	// Handler event
	virtual int HandleEvent(Event * Evt);
};

#endif
