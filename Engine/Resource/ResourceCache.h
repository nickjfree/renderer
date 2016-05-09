#ifndef __RESOURCELOADER__
#define __RESOURCELOADER__


/*
	Resource Loader
*/

#include "Core\System.h"
#include "Container\HashMap.h"
#include "Resource.h"
#include "IO\FileLoader.h"
#include "Core\ObjectFactory.h"
#include "IO\ResourceLoader.h"

#define CACHE_SIZE 8192


class ResourceCache: public System
{
	OBJECT(ResourceCache);
private:
	FileMappingLoader * Loader;
	// resource cache
	HashMap<String, Resource *, CACHE_SIZE> Resources;
	// registered resource types
	HashMap<String, ObjectFactory *> ResourceFactorys;
	// registered resource loaders
	HashMap<String, ResourceLoader*> ResourceLoaderFac;
private:
	void DoAsyncLoadResource(String& URL, Resource * Caller, Variant& Parameter);
public:
	ResourceCache(Context * context);
	virtual ~ResourceCache();
	virtual int Initialize();
	int AsyncLoadResource(String& URL, Resource * Caller, Variant& Parameter);
	template <class T> int RegisterResource();
	template <class T> int RegisterLoader();
	Resource * CreateResource(String& URL);
	template <class T> 	inline  T* Get(String& URL) { return (T*)GetResource(URL); }
	Resource * GetResource(String& URL);
};

#endif

