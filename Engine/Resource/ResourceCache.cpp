#include "ResourceCache.h"
#include "Tasks\WorkQueue.h"
#include "Resource\ResourceTask.h"
#include "Rendering\H3DMesh.h"
#include "Resource\FileMappingLoader.h"
#include "IO\FileLoader.h"
#include "Rendering\Material.h"
#include "Rendering\Texture.h"
#include "Rendering\Shader.h"
#include "Resource\Level.h"
#include "Animation\Skeleton.h"
#include "Animation\BlendShape.h"





ResourceCache::ResourceCache(Context * context) : System(context), Loader(NULL)
{
    Loader = new FileMappingLoader(context);
}


ResourceCache::~ResourceCache()
{
    if (Loader) {
        delete Loader;
    }
}

void ResourceCache::DoAsyncLoadResource(const String& URL, Resource * Caller, Variant& Parameter) {
    // create resource from url
	auto iter = Resources.Find(URL);
	Resource * resource = nullptr;
	if (iter != Resources.End()) {
		resource = *iter;
	} else {
		resource = CreateResource(URL);
	}
    if (resource) {
        WorkQueue * Queue = context->GetSubsystem<WorkQueue>();
        resource->SetAsyncStatus(Resource::S_LOADING);
        if (Caller) {
            resource->AddOwner(Caller, Parameter);
        }
        Resources[URL] = resource;
        ResourceTask * task = ResourceTask::Create();
        task->resource = resource;
        task->Param = Parameter;
		task->cache = this;
		task->Unload = false;
        Queue->QueueTask(task);
    }
}

int ResourceCache::AsyncLoadResource(const String& URL, Resource * Caller, Variant& Param) {
    auto Iter = Resources.Find(URL);
    if (Iter != Resources.End()) {
        // found
        Resource * sub = *Iter;
        if (Caller) {
            if (sub->GetAsyncStatus() == Resource::S_ACTIVED) {
				// already loaded, just notify the caller and add owner
				sub->AddOwner(Caller, Param);
                Caller->OnSubResource(Resource::RM_LOAD, sub, Param);
			} else if (sub->GetAsyncStatus() == Resource::S_LOADING) {
				// loading nothing to be done, add owner
				sub->AddOwner(Caller, Param);
			} else if (sub->GetAsyncStatus() == Resource::S_DESTORYED) {
				printf("cache loading %s\n", URL.ToStr());
				DoAsyncLoadResource(URL, Caller, Param);
			} else if (sub->GetAsyncStatus() == Resource::S_UNLOADING) {
				// resource is UNLOADING, but we want it to be S_ACTIVED
				auto pending = new Resource::PendingLoad();
				pending->Caller = Caller;
				pending->Param = Param;
				pending->Message = Resource::RM_LOAD;
				sub->PendingOperations.Insert(pending);
			}
        }
    }
    else {
        // load this resource
        // TODO : check resource usage,and free some resource
        printf("cache loading %s\n", URL.ToStr());
        DoAsyncLoadResource(URL, Caller, Param);
        // 
    }
    return 0;
}

int ResourceCache::AsyncUnLoadResource(const String& URL, Resource* Caller, Variant& Parameter) {
	auto iter = Resources.Find(URL);
	if (iter != Resources.End()) {
		// found
		Resource* sub = *iter;
		if (sub->ResourceType == Resource::R_SHADER) {
			// do not unload shaders
			sub->RemoveOwner(Caller);
			return 1;
		}
		if (sub->GetAsyncStatus() == Resource::S_ACTIVED) {
			// resource actived, unload it
			printf("cache unloading %s\n", URL.ToStr());
			DoAsyncUnLoadResource(sub, Caller, Parameter);
		} else if (sub->GetAsyncStatus() == Resource::S_UNLOADING || sub->GetAsyncStatus() == Resource::S_DESTORYED) {
			sub->RemoveOwner(Caller);
			// already unloading nothing to be done
		} else if (sub->GetAsyncStatus() == Resource::S_LOADING) {
			// resource is LOADING, but we want it to be DESTORYED
			auto pending = new Resource::PendingLoad();
			pending->Caller = Caller;
			pending->Param = Parameter;
			pending->Message = Resource::RM_UNLOAD;
			sub->PendingOperations.Insert(pending);
		}

	}
	// not found, do nothing
	return 0;
}

void ResourceCache::DoAsyncUnLoadResource(Resource * resource, Resource* Caller, Variant& Parameter) {
	// create resource from url
	WorkQueue* Queue = context->GetSubsystem<WorkQueue>();
	// remove owner
	resource->RemoveOwner(Caller);
	//  check dependences
	if (resource->Owner.Empty()) {
		resource->SetAsyncStatus(Resource::S_UNLOADING);
		ResourceTask* task = ResourceTask::Create();
		task->resource = resource;
		task->Param = Parameter;
		task->cache = this;
		task->Unload = true;
		Queue->QueueTask(task);
	}	
}

template <class T> int ResourceCache::RegisterResource() {
    ObjectFactory * Factory = new ObjectFactoryImp<T>();
    ResourceFactorys[T::GetBaseTypeNameStatic()] = Factory;
    return 0;
}



template <class T> int ResourceCache::RegisterLoader() {
    ResourceLoader * Loader = new T(context);
    ResourceLoaderFac[T::GetTypeNameStatic()] = Loader;
    return 0;
}


Resource * ResourceCache::CreateResource(const String& URL) {
    HashMap<String, ObjectFactory *>::Iterator Iter;
    String Paths[3];
    String& Type = Paths[0];
    String& Pack = Paths[1];
    String& File = Paths[2];
    URL.Split('\\', Paths, 3);
    Iter = ResourceFactorys.Find(Type);
    if (Iter != ResourceFactorys.End()) {
        ObjectFactory * Factorty = *Iter;
        Resource * resource = (Resource*)Factorty->CreateObject(context);
        resource->Pack = Pack;
        resource->File = File;
        resource->URL = URL;
        resource->Type = Type;
        // set loader 
        HashMap<String, ResourceLoader*>::Iterator LIter;
        LIter = ResourceLoaderFac.Find(resource->GetLoaderTypeName());
        if (LIter != ResourceLoaderFac.End()) {
            ResourceLoader * Loader = *LIter;
            resource->Loader = Loader;
        }
        return resource;
    }
    else {
        // ERROR unknow resource
        return NULL;
    }
}

void ResourceCache::RemoveResource(Resource * resource) {
	// remove it
	auto Iter = Resources.Find(resource->GetUrl());
	Resources.Erase(resource->GetUrl());
	delete resource;
}

Resource *  ResourceCache::GetResource(const String& URL) {
    HashMap<String, Resource*, CACHE_SIZE>::Iterator Iter;
    Iter = Resources.Find(URL);
    if (Iter != Resources.End()) {
        // found
        return *Iter;
    }
    else {
        return 0;
    }
}

int ResourceCache::Initialize() {
    // register resource
    RegisterResource<Level>();
    RegisterResource<H3DMesh>();
    RegisterResource<Material>();
    RegisterResource<Texture>();
    RegisterResource<Shader>();
    RegisterResource<Skeleton>();
    RegisterResource<Animation>();
    RegisterResource<BlendShape>();
    // register resource loader
    RegisterLoader<FileMappingLoader>();
    RegisterLoader<FileLoader>();
    return  0;
}