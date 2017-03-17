#ifndef __EVENT__
#define __EVENT__

#include "Shortcuts.h"
#include "Allocater.h"
#include "../Container/Dict.h"
#include "../Container/Vector.h"
#include "../Container/RecyclePool.h"
#include "Core\Str.h"



#define MAX_EVENT_PARAM 4

/*
  Game Event
*/

class Event
{
	DECLAR_ALLOCATER(Event);
	DECLAR_RECYCLE(Event);
public:
	int EventId;
	Dict EventParam;
public:
	Event();
	virtual ~Event();
	Variant& operator[] (String& buff);
};

/*
 Event Pool
*/


class EventPool {
	
};


#endif
