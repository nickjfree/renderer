#ifndef __PARTITION__
#define __PARTITION__

#include "Scene\component.h"


/*
	Partition method for rendering
*/

class PartitionMethod : public Component
{
public:
	PartitionMethod(Context * context);
	virtual ~PartitionMethod();
};

#endif