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
	int Index;
	int AsyncStatus;
	int References;
	ResourceLoader * Loader;
	// the parent resource who is reference this one
	List<Resource> Owner;
	// pending subresources loading
	Dict Dependencies;
	// Dependency count
	int DepCount;
public:
	DECLAR_ALLOCATER(Resource);
	enum Type{
		R_MESH,
		R_MATERIAL,
		R_SKELETON,
		R_RENDERTARET,
		R_TEXTURE,
		R_SHADER
	};
	enum Status {
		S_LOADING,
		S_ACTIVED,
		S_UNLOADING
	};

	enum Message {
		RM_LOAD,
		RM_UNLOAD
	};
	int ResourceType;
public:
	Resource(Context* context);
	virtual ~Resource();
	void SetAsyncStatus(int Status) { AsyncStatus = Status; };
	int GetAsyncStatus() { return AsyncStatus; };
	void SetLoader(ResourceLoader * loader) { Loader = loader; };
	void AddOwner(Resource * Owner);
	int NotifyOwner(int Message, Variant& Param);
	void SetUrl(String& URL);
	String& GetUrl() { return URL; };
	virtual Deserializer AsyncLoad();
	virtual int OnSerialize(Deserializer& serializer) { return 0; };
	virtual int OnLoadComplete(Variant& Data) { return 0; };
	virtual int OnCreateComplete(Variant& Data) { return 0; };
	virtual int AsyncUnLoad(){ return 0; };
	virtual int OnDestory(Variant& Data) { return 0; };
	virtual int OnUnLoadComplete(Variant& Data) { return 0; };
	virtual int OnSubResource(int Message, Resource * Sub, Variant& Param) { return 0; };
};

#endif