#ifndef __EVENT__
#define __EVENT__

#include "Shortcuts.h"
#include "Allocater.h"
#include "../Container/Dict.h"
#include "../Container/Vector.h"
#include "../Container/RecyclePool.h"
#include "Core\Str.h"



/*
    Event codes
*/
#define EV_NODE_ADD    300
#define EV_NODE_REMOVE 301

#define EV_LEVEL_LOAD   400
#define EV_LEVEL_UNLOAD 401





#define MAX_EVENT_PARAM 4

/*
  Game Event
*/




#define USER_EVENT 1024


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
