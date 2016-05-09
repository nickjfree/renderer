#ifndef __RESOURCE_MANAGER__
#define __RESOURCE_MANAGER__


//#include "ObjectContainer.h"
#include "Entity.h"
#include "ResourceContainer.h"
#include "RenderResource.h"
#include "CreationQueue.h"
#include "Render.h"
#include "CreationCommand.h"



#define TEXTURE     1
#define MESH        2
#define BUFFER      3



#define TEXTURE_SACLES 5               // 64 128  256 512 1024
#define RESOURCE_SCALES 8         //  resource scales


#define MAX_RESOURCE_TYPE  8       // resource types


#define RES_MASK        (0x00ffffff)
#define RES_TYPE_SHIFT   24



using Render::CCreationQueue;
using namespace Render;
using Render::CRender;
using SceneManager::CEntity;

namespace ResourceManager{


typedef struct LoadingCMD
{
	int             ID;
	LoadingCMD  *  NextBatch;
	CRenderResource * Resource;
	int Type;
	bool  LoadOver;
	bool  UnLoad;
}LoadingCMD;

typedef struct FreeResource
{
	int   ID;     // the ID of Container;
	int   ResID;  // the ID of Resource
}FreeResource;


class CResourceLoader
{
protected:
	CRenderResource * m_FreeResource[RESOURCE_SCALES];

protected:
	virtual CRenderResource * MatchFreeResource(int Scale, CRenderResource * Resource);
	int ToScale(int Scale, CRenderResource * Resource);
	int RemoveFromFreeList(CRenderResource * Resource);

public:
	CResourceLoader();
	~CResourceLoader();
	virtual CRenderResource * AllocResource(ResourceDesc * Desc);
	virtual CRenderResource * GetResource(int ID);
	virtual int UseResource(int ID){ return -1;}
	virtual int LoadResource(int ID) { return -1;}
	virtual int UnLoadResource(int ID) { return -1; }
	virtual int GetFreeReourceID(CRenderResource * Resource);
};

class CResourceManager
{
private:
	//texture pool
	CResourceContainer<CTexture> m_TexturePool;
	//mesh pool
	CResourceContainer<CMesh>    m_MeshPool;
	//Boneinfo pool
	CResourceContainer<CBoneInfo> m_BoneInfoPool;  

	// Resource Loader, Factory Pattern
	CResourceLoader *           m_ResourceLoaders[MAX_RESOURCE_TYPE];


	static CResourceManager * m_Manager;
	//CCreationQueue;
	//CCreationQueue<TextureCMDBatch> m_TextureCompleteQueue;
	//CCreationQueue<MeshCMDBatch>    m_MeshCompleteQueue;
	//CCreationQueue<BoneCMDBatch>    m_BoneCompleteQueue;

	// Creation queue
	CCreationQueue<CreationCommand>     m_CompleteQueue;

	// Loading Queue
	CCreationQueue<LoadingCMD>      m_LoadingQueue;
	CCreationQueue<LoadingCMD>      m_LoadingOverQueue;
	HANDLE                          m_LoadingEvent;
	HANDLE                          m_LoadingThread;
	HANDLE                          m_LoadingLock;


	// Batch Container
	//CResourceContainer<TextureCMDBatch> m_TextureBatchContainer;
	//CResourceContainer<MeshCMDBatch>    m_MeshBatchContainer;
	//CResourceContainer<BoneCMDBatch>    m_BoneBacthContainer;
	CResourceContainer<CreationCommand>      m_ResourceCMDContainer;

	// LoadingCMD Container
	CResourceContainer<LoadingCMD>      m_LoadingCMDContainer;

	// Free Geometry Buffer and Texture
	CResourceContainer<FreeResource>             m_FreeGeometry;
	CResourceContainer<FreeResource>             m_FreeTexture[TEXTURE_SACLES];


	//Craeting Batchs
	// dynamic resource
	//TextureCMDBatch * m_TextureCreatingBatch;
	//MeshCMDBatch *    m_MeshCreatingBatch;
	//BoneCMDBatch *    m_BoneCreatingBatch;

private:
	//private mthod


	//Get Manager
public:
	inline static CResourceManager * GetResourceManager(void){return CResourceManager::m_Manager;};

public:
	CResourceManager(void);
	~CResourceManager(void);

	//register and creation
	int RegisterTexture(wchar_t ** FileList);	
	int RegisterStreamTexture(char * File, char * TextureName, int Type); 
	int RegisterMesh(int MemoryType);
	int RegisterStreamMesh(int MemoryType,char * File,char * ModelName,int MeshIndex);
	int SetMeshData(int MeshID, void * VBuffer, int VSize, WORD * IBuffer, int INum);
	int RegisterBone(char * File,char * ModelName,int MeshIndex);

	//register resource, load resource, using resource
	int RegisterResource(int Type, ResourceDesc * ResDesc);
	int RegisterCallback(int ID, h3d_callback callback, void * callback_data);
	int LoadResource(int ID);
	int UnLoadResource(int ID);
	void RegisterResourceLoader(int Type, CResourceLoader * Loader) {m_ResourceLoaders[Type] = Loader;};
	CResourceLoader * GetResourceLoader(int Type) { return m_ResourceLoaders[Type];}
	int UsingResource(int ID);

public:
	int CreateResource(int Type, CRenderResource * Resource);

	int CompleteResource(CreationCommand * CMD);
	//int CompleteMesh(MeshCMDBatch * Batch);
	//int CompleteBone(BoneCMDBatch * Batch);

	// process completion
private:
	int ProcessCreationComplete(void);


	//Batch Alloc() and Free()
private:
	//TextureCMDBatch * AllocTextureCreationBatch(void);
	//MeshCMDBatch * AllocMeshCreationBatch(void);
	//BoneCMDBatch * AllocBoneCreationBatch(void);
	//int FreeTextureCreationBatch(TextureCMDBatch * Batch);
	//int FreeMeshCreationBatch(MeshCMDBatch * Batch);
	//int FreeBoneCreationBatch(BoneCMDBatch * Batch);

private:
	// // Resource load over create it now
	int ProcessCreation(void);
	// return free geomety id in VRAM
	int FindFreeMesh(CMesh * LoadingMesh);
	int FindFreeTexture(CTexture * LoadingTexture);
	int RemoveFromFreeMesh(CMesh * Mesh);
	int RemoveFromFreeTexture(CTexture * Texture);
	int LoadResource(int Type, CRenderResource * Resource);
	int UnLoadResource(int Type, CRenderResource * Resource);
	int AddLoadingCMD(LoadingCMD * CMD);
	LoadingCMD * GetLoadingOver(void);

public:
	int Update(float DeltaTime);
	int UsingTexture(int ResourceID);
	int UsingMesh(int ResourceID);
	int UsingBone(int ResourceID);
	int GetBoneFrams(int ResourceID);
	int GetBoneNum(int ResourceID);
	int GetMeshToEntity(CEntity * Entity);
	Matrix4x4 * GetBoneData(int ResourceID);
	static int __stdcall LoadingThread(CResourceManager * ResManager);
	LoadingCMD * GetLoadingCMD(void);
	int AddLoadingOver(LoadingCMD * CMD);
	int ToFreeMesh(CMesh * Mesh);
	int ToFreeTexture(CTexture * Texture);
	int LoadMesh(int ID);
	int UnLoadMesh(int ID);
	int LoadStreamTextue(int ID);
	int UnLoadStreamTexture(int ID);
	
};


}

#endif
