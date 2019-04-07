#include "ResourceTask.h"

USING_ALLOCATER(ResourceTask);
USING_RECYCLE(ResourceTask);

ResourceTask::ResourceTask()
{
}


ResourceTask::~ResourceTask()
{
}


int ResourceTask::Work() {
	// load file into memorty
    
	Deserializer deserializer = resource->AsyncLoad();
	// serialize and 
	resource->OnSerialize(deserializer);
    // pass raw data ownership to resource
    resource->SetDeserializer(std::move(deserializer));
	// on load complete
	resource->OnLoadComplete(Param);
	return 0;
}

int ResourceTask::Complete() {

	//resource->OnLoadComplete(Param);
	resource->SetAsyncStatus(Resource::S_ACTIVED);
	resource->OnCreateComplete(Param);
	return 0;
}