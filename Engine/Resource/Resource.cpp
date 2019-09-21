#include "Resource.h"


USING_ALLOCATER(Resource)

Resource::Resource(Context* context) : EventNode(context), DepCount(0)
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

void Resource::AddOwner(Resource * owner) {
    Owner.Insert(owner);
}

void Resource::RemoveOwner(Resource* owner) {
	Owner.Remove(owner);
}

int Resource::NotifyOwner(int Message, Variant& Param) {
	for(auto iter = Owner.Begin();  iter != Owner.End(); iter++) {
        Resource * owner = *iter;
        owner->OnSubResource(Message, this, Param);
    }
    return 0;
}

