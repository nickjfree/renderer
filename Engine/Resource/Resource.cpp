#include "Resource.h"
#include "ResourceCache.h"


USING_ALLOCATER(Resource)
USING_ALLOCATER(Resource::CallerNotice)
USING_ALLOCATER(Resource::PendingLoad)


Resource::Resource(Context* context) : EventNode(context), DepCount(0), AsyncStatus(S_NONE)
{
}


Resource::~Resource()
{
}


Deserializer Resource::AsyncLoad() {
    printf("asyncload %s\n", URL.ToStr());
    return Loader->GetDeserializer(URL);
}

void Resource::SetUrl(String& url) {
    URL = url;
    String Paths[3];
    URL.Split('\\', Paths, 3);
    Pack = Paths[1];
    File = Paths[2];
}

void Resource::AddOwner(Resource * owner, Variant& Parameter) {
	auto notice = new CallerNotice();
	notice->Caller = owner;
	notice->Param = Parameter;
    Owner.Insert(notice);
}

void Resource::RemoveOwner(Resource* owner) {
	for (auto iter = Owner.Begin(); iter != Owner.End(); iter++) {
		auto notice = *iter;
		if (notice->Caller == owner) {
			Owner.Remove(notice);
			delete notice;
			return;
		}
	}
}

int Resource::NotifyOwner(int Message) {
	for(auto iter = Owner.Begin();  iter != Owner.End(); iter++) {
		auto notice = *iter;
        Resource * owner = notice->Caller;
        owner->OnSubResource(Message, this, notice->Param);
		owner->UpdateStatus();
    }
    return 0;
}


int Resource::UpdateStatus() {
	if (DepCount == 0) {
		SetAsyncStatus(Resource::S_ACTIVED);
		NotifyOwner(Resource::RM_LOAD);
		// handle pending
		HandlePendingLoad();
	}
	return 0;
}


int Resource::HandlePendingLoad() {

	// get cache
	auto cache = context->GetSubsystem<ResourceCache>();
	// handle pendings
	auto iter = PendingOperations.Begin();
	while (iter != PendingOperations.End()) {
		auto pending = *iter;
		if (pending->Message == Resource::RM_LOAD && AsyncStatus != Resource::S_UNLOADING ) {
			cache->AsyncLoadResource(GetUrl(), pending->Caller, pending->Param);
		} else if (pending->Message == Resource::RM_UNLOAD && AsyncStatus == Resource::S_LOADING) {
			cache->AsyncUnLoadResource(GetUrl(), pending->Caller, pending->Param);
		} else {
			break;
		}
		// operation applied, remove it
		PendingOperations.Remove(pending);
		// free
		delete pending;
		return 1;
	}
	return 0;
}