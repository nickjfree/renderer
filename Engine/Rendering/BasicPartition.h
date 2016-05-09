#ifndef __BASIC_P_METHOD__
#define __BASIC_P_METHOD__

#include "PartitionMethod.h"
#include "BasicCullingImp.h"

/*
	Basci Partition method
*/
class BasicPartition : public PartitionMethod
{
private:
	BasicCullingImp * BasicCulling;
public:
	BasicPartition(Context * context);
	virtual ~BasicPartition();
	virtual int OnAttach(GameObject * GameObj);
	virtual int HandleEvent(Event * Evt);
};

#endif
