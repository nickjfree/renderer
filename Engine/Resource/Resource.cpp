#include "Resource.h"


USING_ALLOCATER(Resource)

Resource::Resource(Context* context) : EventNode(context), References(0), DepCount(0)
{
}


Resource::~Resource()
{
}


Deserializer Resource::AsyncLoad() {
	printf("asyncload %s\n", (char*)URL);
	return Loader->GetDeserializer(URL);
	//void * Base = Loader->GetMappingBase(Pack);
	//void * Header = Loader->GetFileHeader(Base, File);
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

int Resource::NotifyOwner(int Message, Variant& Param) {
	List<Resource>::Iterator Iter = Owner.Begin();
	while (Iter != Owner.End()) {
		Resource * owner = *Iter;
		owner->OnSubResource(Message, this, Param);
		Owner.Remove(Iter);
		Iter = Owner.Begin();
	}
	return 0;
}

