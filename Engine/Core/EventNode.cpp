#include "EventNode.h"



USING_ALLOCATER(EventRegistry);



EventNode::EventNode(Context * context_) :Object(context_) {
	/*EventChannel = new HashMap<int, List<EventNode>>();*/
}


EventNode::~EventNode() {
	/*delete EventChannel;*/
}


int EventNode::SubscribeTo(EventNode * Hub, int EventId) {
	Hub->AddEventHandler(EventId, this);
	EventRegistry * Item = new EventRegistry();
	// insert to publishers
	Item->Link.Owner = Item;
	Item->Node = Hub;
	Item->EventId = EventId;
	Item->Link.InsertAfter(&Publishers);
	return 0;
}

int EventNode::Subscribe(int Event, String& Callback) {
	return context->SubscribeFor(this, Event);
}

int EventNode::UnSubscribe(EventNode * Hub, int EventId) {
	Hub->RemoveEventHandler(EventId, this);
	// remove from publishers
	LinkList<EventRegistry>::Iterator Iter;
	for (Iter = Publishers.Begin(); Iter != Publishers.End();) {
		EventRegistry * Item = *Iter;
		Iter++;
		if (Item->EventId == EventId && Item->Node == Hub) {
			Item->Link.Remove();
			delete Item;
		}
	}
	return 0;
}


int EventNode::AddEventHandler(int EventId, EventNode* Handler) {
	// just put hanlder to certain Channel Map
	if (!Handler) {
		return -1;
	}
	Handler->AddRef();
	EventChannel[EventId].Insert(Handler);
	EventRegistry * Item = new EventRegistry();
	Item->Link.Owner = Item;
	Item->Node = Handler;
	Item->EventId = EventId;
	Item->Link.InsertAfter(&Subscribers);
	return 0;
}

int EventNode::RemoveEventHandler(int EventId, EventNode* Handler) {
	if (!Handler) {
		return -1;
	}
	auto Iter = EventChannel.Find(EventId);
	if (Iter == EventChannel.End()) {
		return -1;
	}
	else {
		List<EventNode> &Handlers = *Iter;
		for (auto HandlerIter = Handlers.Begin(); HandlerIter != Handlers.End(); HandlerIter++) {
			if (*HandlerIter == Handler) {
				Handlers.Remove(HandlerIter);
				Handler->DecRef();
				return 0;
			}
		}
	}
	// remove from subscribers
	LinkList<EventRegistry>::Iterator IterSub;
	for (IterSub = Subscribers.Begin(); IterSub != Subscribers.End();) {
		EventRegistry * Item = *IterSub;
		IterSub++;
		if (Item->EventId == EventId && Item->Node == Handler) {
			Item->Link.Remove();
			delete Item;
		}
	}
	return -1;
}



int EventNode::SendEvent(EventNode * Receiver, Event * Evt) {
	// just handle this event with receiver for SendEvent
	return Receiver->HandleEvent(Evt);
}

// Handler event
int EventNode::HandleEvent(Event * Evt) {
	// print test
	int Result = -1;
	auto Iter = EventChannel.Find(Evt->EventId);
	if (Iter == EventChannel.End()) {
		return -1;
	}
	else{
		List<EventNode> &Handlers = *Iter;
		for (auto HandlerIter = Handlers.Begin(); HandlerIter != Handlers.End(); HandlerIter++) {
			Result = (*HandlerIter)->HandleEvent(Evt);
		}
	}
	return 0;
}

void EventNode::DisableEvent() {
	LinkList<EventRegistry>::Iterator Iter;
	// clear all subscibers
	for (Iter = Subscribers.Begin(); Iter != Subscribers.End();) {
		EventRegistry * Item = *Iter;
		Iter++;
		Item->Node->UnSubscribe(this, Item->EventId);
	}
	// clear all publishers
	for (Iter = Publishers.Begin(); Iter != Publishers.End();) {
		EventRegistry * Item = *Iter;
		Iter++;
		UnSubscribe(Item->Node, Item->EventId);
	}

}