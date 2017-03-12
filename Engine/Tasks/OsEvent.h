#ifndef __OS_EVENT__
#define __OS_EVENT__

#include "windows.h"
#include "Core\Allocater.h"
#include "Container\RecyclePool.h"
#include "Core\Shortcuts.h"

/*
	win32 event
*/

#define EVENT_MAX_JOIN 256

class OsEvent
{
	DECLAR_ALLOCATER(OsEvent);
	DECLAR_RECYCLE(OsEvent);
	friend 	OsEvent;
private:
	HANDLE hEvent;
public:
	OsEvent();
	OsEvent(bool manual);
	~OsEvent();
	void Pulse();
	void Set();
	void Reset();
	static int Join(int Count, OsEvent ** Events, bool reset);

};

#endif
