#include "EventNode.h"


EventNode::EventNode(Context * context_) :Object(context_)
{
}


EventNode::~EventNode()
{
}


int EventNode::SubscribeTo(EventNode * Hub, int EventId) 
{
	Hub->AddEventHandler(EventId, this);
	return 0;
}


int EventNode::UnSubscribe(EventNode * Hub, int EventId) 
{
	Hub->RemoveEventHandler(EventId, this);
	return 0;
}


int EventNode::AddEventHandler(int EventId, EventNode* Handler)
{
	// just put hanlder to certain Channel Map
	if (!Handler) {
		return -1;
	}
	Handler->AddRef();
	EventChannel[EventId].Insert(Handler);
	return 0;
}

int EventNode::RemoveEventHandler(int EventId, EventNode* Handler)
{
	if (!Handler) {
		return -1;
	}
	HashMap<int, List<EventNode>>::Iterator Iter = EventChannel.Find(EventId);
	if (Iter == EventChannel.End()) {
		return -1;
	}
	else {
		List<EventNode> &Handlers = *Iter;
		List<EventNode>::Iterator HandlerIter;
		for (HandlerIter = Handlers.Begin(); HandlerIter != Handlers.End(); HandlerIter++) {
			if (*HandlerIter == Handler) {
				Handlers.Remove(HandlerIter);
				Handler->DecRef();
				return 0;
			}
		}

	}
	return -1;
}



int EventNode::SendEvent(EventNode * Receiver, Event * Evt)
{
	// just handle this event with receiver for SendEvent
	return Receiver->HandleEvent(Evt);
}

// Handler event
int EventNode::HandleEvent(Event * Evt)
{
	// print test
	int Result = -1;
	HashMap<int, List<EventNode>>::Iterator Iter = EventChannel.Find(Evt->EventId);
	if (Iter == EventChannel.End()) {
		return -1;
	}
	else{
		List<EventNode> &Handlers = *Iter;
		List<EventNode>::Iterator HandlerIter;
		for (HandlerIter = Handlers.Begin(); HandlerIter != Handlers.End(); HandlerIter++) {
			Result = (*HandlerIter)->HandleEvent(Evt);
		}
	}
	return 0;
}