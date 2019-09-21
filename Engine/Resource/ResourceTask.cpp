#include "ResourceTask.h"

USING_ALLOCATER(ResourceTask);
USING_RECYCLE(ResourceTask);

ResourceTask::ResourceTask(): Unload(false)
{
}


ResourceTask::~ResourceTask()
{
}


int ResourceTask::Work() {
	if (!Unload) {
		// load file into memorty   
		Deserializer deserializer = resource->AsyncLoad();
		// serialize and 
		resource->OnSerialize(deserializer);
		// pass raw data ownership to resource
		resource->SetDeserializer(std::move(deserializer));
		// on load complete
		resource->OnLoadComplete(Param);
	} else {
		// do unload task
		resource->AsyncUnLoad();

	}
    return 0;
}

int ResourceTask::Complete() {
	if (!Unload) {
		//resource->OnLoadComplete(Param);
		resource->SetAsyncStatus(Resource::S_ACTIVED);
		resource->OnCreateComplete(Param);
	} else {
		// unload complete
		resource->SetAsyncStatus(Resource::S_DESTORYED);
		// on destroy
		resource->OnDestroy(Param);
		// remove resource from cache
		cache->RemoveResource(resource);
		// delete resource
		delete resource;
	}

    return 0;
}