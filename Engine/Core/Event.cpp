#include "Event.h"


USING_ALLOCATER(Event);
USING_RECYCLE(Event);


Event::Event()
{
}


Event::~Event()
{
}

Variant& Event::operator[] (char * buff) {
	return EventParam[buff];
}
