#include "ResourceTask.h"

USING_ALLOCATER(ResourceTask);
USING_RECYCLE(ResourceTask);

ResourceTask::ResourceTask() : Unload(false)
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

	}
	else {
		// do unload task
		resource->AsyncUnLoad();
	}
	return 0;
}

int ResourceTask::Complete() {
	if (!Unload) {
		// save owner param
		resource->OnCreateComplete(Param);
		// update status, notify owner if depcount==0
		resource->UpdateStatus();

		printf_s("resource %s created\n", resource->GetUrl().ToStr());
	}
	else {
		// unload complete
		resource->SetAsyncStatus(Resource::S_DESTORYED);
		// handle pending load
		auto has_pending = resource->HandlePendingLoad();
		// on destroy
		resource->OnDestroy(Param);
		// remove resource from cache
		if (!has_pending) {
			cache->RemoveResource(resource);
		}
		printf_s("resource %s destroyed\n", resource->GetUrl().ToStr());
	}

	return 0;
}