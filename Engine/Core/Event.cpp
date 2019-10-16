#include "Event.h"


USING_ALLOCATER(Event);
USING_RECYCLE(Event);


Event::Event()
{
}


Event::~Event()
{
}

Variant& Event::operator[] (String& buff) {
	return EventParam[buff];
}
