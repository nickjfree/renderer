#ifndef __BASIC_P_METHOD__
#define __BASIC_P_METHOD__

#include "PartitionMethod.h"
#include "BasicCullingImp.h"

/*
	Basci Partition method
*/
class BasicPartition : public PartitionMethod
{
	DECLARE_ALLOCATER(BasicPartition)
private:
	BasicCullingImp* BasicCulling;
public:
	BasicPartition(Context* context);
	virtual ~BasicPartition();
	virtual int OnAttach(GameObject* GameObj);
	virtual int HandleEvent(Event* Evt);
};

#endif
