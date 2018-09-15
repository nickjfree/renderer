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
	if (resource) {
		int a = 100;
	}

	Deserializer deserializer = resource->AsyncLoad();
	// serialize

	resource->OnSerialize(deserializer);
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