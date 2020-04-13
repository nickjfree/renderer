#ifndef __RESOURCE__
#define __RESOURCE__

#include "Core\Object.h"
#include "Resource\FileMappingLoader.h"
#include "Core\EventNode.h"
#include "IO\Deserializer.h"
#include "Container\LinkList.h"
#include "Container\List.h"
#include "Core\Variant.h"


/*
	Base class of all the resource in game.
	Mesh
	Skeletion
	Shader, Texture
	Material
	Level.
*/


class ResourceCache;


class Resource : public EventNode
{
	OBJECT(Resource);
	BASEOBJECT(Resource);
	LOADEROBJECT(FileMappingLoader);
	friend ResourceCache;
protected:
	// resource url
	String URL;
	String Type;
	String Pack;
	String File;
	// resource index in level
	int Index;
	// resource current state 
	int AsyncStatus;
	// resource loader
	ResourceLoader* Loader;
	// pending subresources loading
	Dict Dependencies;
	// Dependency count
	int DepCount;
	// serializer and deserializer
	Deserializer DeSerial;

public:
	DECLAR_ALLOCATER(Resource);
	// resource type
	enum Type {
		R_MESH,
		R_MATERIAL,
		R_SKELETON,
		R_RENDERTARET,
		R_TEXTURE,
		R_SHADER,
		R_SHADER_LIBRARY,
		R_ANIMATION,
		R_BLEDNSHAPE,
	};
	// resource state
	enum Status {
		S_NONE,
		S_LOADING,
		S_ACTIVED,
		S_UNLOADING,
		S_DESTORYED,
	};
	// loading message
	enum Message {
		RM_LOAD,
		RM_UNLOAD,
	};
	// pending load	
	typedef struct PendingLoad
	{

		DECLAR_ALLOCATER(PendingLoad)
			Resource* Caller;
		Resource::Message Message;
		Variant Param;

	}PendingLoad;
	// callers

	typedef struct CallerNotice
	{
		DECLAR_ALLOCATER(CallerNotice)
			Resource* Caller;
		Variant Param;
	}CallerNotice;
	// resource type
	int ResourceType;

protected:
	// the parent resource who is reference this one
	List<CallerNotice> Owner;
	// pending loading operation
	List<PendingLoad> PendingOperations;

public:
	Resource(Context* context);
	virtual ~Resource();
	// set resource status
	void SetAsyncStatus(int Status) { AsyncStatus = Status; };
	// get resource status
	int GetAsyncStatus() { return AsyncStatus; };
	// set resource loader
	void SetLoader(ResourceLoader* loader) { Loader = loader; };
	// add parent resource, resource that depende on this resource
	void AddOwner(Resource* Owner, Variant& Parameter);
	// remove parent
	void RemoveOwner(Resource* Owner);
	// notify owner 
	int NotifyOwner(int Message);
	// set resource url
	void SetUrl(String& URL);
	// set resource deserializer
	void SetDeserializer(Deserializer&& deserializer) { DeSerial = std::move(deserializer); };
	// get url
	const String& GetUrl() const { return URL; };
	// load resource using loader
	virtual Deserializer AsyncLoad();
	// on raw data(worker thread)
	virtual int OnSerialize(Deserializer& serializer) { return 0; };
	// on raw data parse complete(worker thread)
	virtual int OnLoadComplete(Variant& Data) { return 0; };
	// on resource create complete(main thread)
	virtual int OnCreateComplete(Variant& Data) { return 0; };
	// do unload
	virtual int AsyncUnLoad() { return 0; };
	// called when unload finished
	virtual int OnDestroy(Variant& Data) { return 0; };
	// on sub resource create complete(main thread)
	virtual int OnSubResource(int Message, Resource* Sub, Variant& Param) { return 0; };
	// update status, notify owner if depcount==0
	int UpdateStatus();
	// handle pending operations
	int HandlePendingLoad();
};

#endif