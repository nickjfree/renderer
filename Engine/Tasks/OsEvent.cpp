#include "OsEvent.h"

USING_ALLOCATER(OsEvent);
USING_RECYCLE(OsEvent);

OsEvent::OsEvent() {
	hEvent = CreateEventA(0, 1, 0, 0);
}

OsEvent::OsEvent(bool manual) {
	hEvent = CreateEventA(0, manual, 0, 0);
}

OsEvent::~OsEvent()
{
	CloseHandle(hEvent);
}

void OsEvent::Pulse() {
	PulseEvent(hEvent);
}

void OsEvent::Set() {
	SetEvent(hEvent);
}

void OsEvent::Reset() {
	ResetEvent(hEvent);
}

int OsEvent::Join(int Count, OsEvent** Events, bool reset = 1) {
	HANDLE handles[EVENT_MAX_JOIN];
	for (int i = 0; i < Count; i++) {
		handles[i] = Events[i]->hEvent;
	}
	WaitForMultipleObjects(Count, handles, 1, -1);
	if (reset) {
		for (int i = 0; i < Count; i++) {
			Events[i]->Reset();
		}
	}
	return 0;
}
