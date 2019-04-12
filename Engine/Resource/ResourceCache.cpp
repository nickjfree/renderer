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
    Resource * resource = CreateResource(URL);
    if (resource) {
        WorkQueue * Queue = context->GetSubsystem<WorkQueue>();
        resource->SetAsyncStatus(Resource::S_LOADING);
        if (Caller) {
            resource->AddOwner(Caller);
        }
        Resources[URL] = resource;
        ResourceTask * task = ResourceTask::Create();
        task->resource = resource;
        task->Param = Parameter;
        Queue->QueueTask(task);
    }
}

int ResourceCache::AsyncLoadResource(const String& URL, Resource * Caller, Variant& Param) {
    auto Iter = Resources.Find(URL);
    if (Iter != Resources.End()) {
        // found
        Resource * sub = *Iter;
        if (Caller) {
            if (sub->GetAsyncStatus() == Resource::S_LOADING) {
                sub->AddOwner(Caller);
            }
            else {
                Caller->OnSubResource(Resource::RM_LOAD, sub, Param);
            }
        }
        //SendEvent(Caller, NULL);
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