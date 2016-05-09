#include "StdAfx.h"
#include "ResourceManager.h"
#include "RenderResource.h"
#include "Render.h"


using ResourceManager::CResourceManager;
using ResourceManager::LoadingCMD;
using namespace ResourceManager;

CResourceManager * CResourceManager::m_Manager = NULL;

CResourceManager::CResourceManager(void)
{
	m_Manager = this;
	//m_TextureCreatingBatch = NULL;
	//m_MeshCreatingBatch = NULL;
	//m_BoneCreatingBatch = NULL;
	// init
	m_LoadingEvent = CreateEvent(NULL,true,false,NULL);
	m_LoadingLock =  CreateMutex(NULL,false,NULL);
	//create loading thread
	m_LoadingThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)LoadingThread,this,0,0);

}

CResourceManager::~CResourceManager(void)
{
	CloseHandle(m_LoadingEvent);
}

int ResourceManager::CResourceManager::RegisterTexture(wchar_t ** FileList)
{
	CTexture * pTexture;
	int TextureID;
	int CmdID;
	TextureID = m_TexturePool.AllocResource(&pTexture);
	pTexture->m_ResourceID = TextureID;
	pTexture->SetPermanent(true);
	pTexture->BindFile(FileList);

	LoadingCMD * CMD;
	CmdID = m_LoadingCMDContainer.AllocResource(&CMD);
	CMD->ID = CmdID;
	CMD->Resource = pTexture;
//	AddLoadingCMD(CMD);



	// Create it now
	/*pTexture->IntoVRAM();*/
	return TextureID;
}


int ResourceManager::CResourceManager::RegisterResource(int Type, ResourceDesc * ResDesc)
{
	// the upper 8 bit contains resource type
	int ResourceID = Type << 24;
	CResourceLoader * Loader = m_ResourceLoaders[Type];
	CRenderResource * Resource = Loader->AllocResource(ResDesc);
	Resource->m_Status = RES_OUT;
	ResourceID += Resource->m_ResourceID;
	return ResourceID;
}

int ResourceManager::CResourceManager::LoadResource(int ID)
{
	int Type = ID >> RES_TYPE_SHIFT;
	CResourceLoader * Loader = m_ResourceLoaders[Type];
	CRenderResource * Resource = Loader->GetResource(ID & RES_MASK);
	if (Resource) {
		if (Resource->m_Status == RES_FREE)
		{
			// do not need loading
			Resource->m_Status = RES_USING;
//			RemoveFromFreeMesh(Mesh);
			return 0;
		}
		else
		{
			if (Resource->m_Status == RES_OUT)
			{
				// we need to load this mesh
				Resource->m_Status = RES_LOADING;
				LoadResource(Type, Resource);
			}
		}
		return 0;
	}
	return -1;
}

int ResourceManager::CResourceManager::RegisterCallback(int ID, h3d_callback callback, void * Data) {
	int Type = ID >> RES_TYPE_SHIFT;
	CResourceLoader * Loader = m_ResourceLoaders[Type];
	CRenderResource * Resource = Loader->GetResource(ID & RES_MASK);
	if (Resource) {
		Resource->callback = callback;
		Resource->callback_data = Data;
	}
	return 0;
}

int ResourceManager::CResourceManager::UnLoadResource(int ID)
{
	int Type = ID >> RES_TYPE_SHIFT;
	CResourceLoader * Loader = m_ResourceLoaders[Type];
	CRenderResource * Resource = Loader->GetResource(ID & RES_MASK);
	if (Resource)
	{
		if (Resource->m_Status == RES_LOADING)
		{
			Resource->m_Status = RES_DELAY_FREE;
			return -1;
		}
		if (Resource->m_RAMOnly)
		{
			if (Resource->m_Status == RES_USING)
			{
				UnLoadResource(Type, Resource);
				Resource->m_Status = RES_OUT;
			}
		}
		else
		{
			if (Resource->m_Status != RES_USING)
			{
				return -1;
			}
			UnLoadResource(Type, Resource);
			Resource->m_Status = RES_FREE;
		}
	}
	
	return -1;
	return 0;
}

int ResourceManager::CResourceManager::UsingResource(int ID)
{
	int Type = ID >> RES_TYPE_SHIFT;
	if (Type < 0 || Type >= MAX_RESOURCE_TYPE)
		return -1;
	CResourceLoader * Loader = m_ResourceLoaders[Type];
	return Loader->UseResource(ID & RES_MASK);
}

int ResourceManager::CResourceManager::RegisterStreamTexture(char * File, char * TextureName, int Type)
{
	CTexture * pTexture;
	int TextureID;
	int CmdID;
	TextureID = m_TexturePool.AllocResource(&pTexture);
	pTexture->m_ResourceID = TextureID;
	pTexture->SetPermanent(false);
	pTexture->m_StreamType = Type;
	pTexture->m_Status = RES_OUT;
	pTexture->BindStreamFile(File, TextureName, Type);
	return TextureID;
}

int ResourceManager::CResourceManager::ProcessCreationComplete(void)
{
	CreationCommand * CMD = NULL;
	while (CMD = m_CompleteQueue.RemoveBatch())
	{
		CRenderResource * Resource = CMD->m_Resource;
		Resource->m_ResourceIDVRAM = CMD->CreateId;
		if (Resource->callback) {
			Resource->callback(Resource->callback_data, CMD->CreateId);
		}
		if (Resource->m_Status == RES_DELAY_FREE)
		{
			// put that resource to free pool
		}
		else
		{
			Resource->m_Status = RES_USING;
		}
		m_ResourceCMDContainer.FreeResource(CMD->CmdId);
	}
	return 0;
}

//int ResourceManager::CResourceManager::CreateTexture(CTexture * Texture)
//{
//	if(!m_TextureCreatingBatch)
//	{
//		m_TextureCreatingBatch = AllocTextureCreationBatch();
//		m_TextureCreatingBatch->CMDNum = 0;
//	}
//	else
//	{
//		if(m_TextureCreatingBatch->CMDNum  == 32)
//		{
//			FlushCreationCMD();
//			m_TextureCreatingBatch = AllocTextureCreationBatch();
//			m_TextureCreatingBatch->CMDNum = 0;
//		}	
//	}
//	m_TextureCreatingBatch->CMD[m_TextureCreatingBatch->CMDNum].CreateId = -1;
//	m_TextureCreatingBatch->CMD[m_TextureCreatingBatch->CMDNum].Scale = Texture->m_Scale;
//	m_TextureCreatingBatch->CMD[m_TextureCreatingBatch->CMDNum].StreamType = Texture->m_StreamType;
//	memcpy(m_TextureCreatingBatch->CMD[m_TextureCreatingBatch->CMDNum].FileInMemorey,Texture->m_RAMDatas,8 * sizeof(char *));
//	m_TextureCreatingBatch->CMD[m_TextureCreatingBatch->CMDNum].Id = Texture->m_ResourceID;
//	memcpy(m_TextureCreatingBatch->CMD[m_TextureCreatingBatch->CMDNum].TextureName,Texture->m_Files,8 * sizeof(char *));
//	m_TextureCreatingBatch->CMDNum++;
//	return 0;
//}

//int ResourceManager::CResourceManager::FlushCreationCMD(void)
//{
//	CRender * render = CRender::GetRender();
//	if(m_TextureCreatingBatch)
//	{
//		render->CreateTexture(m_TextureCreatingBatch);	
//	}
//	if(m_MeshCreatingBatch)
//	{
//		render->CreateMesh(m_MeshCreatingBatch);
//	}
//	if(m_BoneCreatingBatch)
//	{
//		render->CreateBone(m_BoneCreatingBatch);
//	}
//	//clear
//	m_MeshCreatingBatch    = NULL;
//	m_TextureCreatingBatch = NULL;
//	m_BoneCreatingBatch    = NULL;
//	return 0;
//}

int ResourceManager::CResourceManager::Update(float DeltaTime)
{
	ProcessCreationComplete();
	ProcessCreation();
	return 0;
}

int ResourceManager::CResourceManager::RegisterMesh(int MemoryType)
{
	CMesh * Mesh;
	int ID = m_MeshPool.AllocResource(&Mesh);
	Mesh->m_ResourceID = ID;
	Mesh->SetPermanent(true);
	Mesh->m_Status = RES_OUT;
	Mesh->m_RAMOnly = MemoryType;
	return ID;
}

int ResourceManager::CResourceManager::RegisterStreamMesh(int MemoryType,char * File,char * ModelName,int MeshIndex)
{
	CMesh * Mesh;
	int ID = m_MeshPool.AllocResource(&Mesh);
	Mesh->m_ResourceID = ID;
	Mesh->SetPermanent(true);
	Mesh->Bind(File,ModelName,MeshIndex);
	Mesh->m_Status = RES_OUT;
	Mesh->m_RAMOnly = MemoryType;
	return ID;
}

int ResourceManager::CResourceManager::SetMeshData(int MeshID, void * VBuffer, int VSize, WORD * IBuffer, int INum)
{
	int Type = MeshID >> RES_TYPE_SHIFT;
	CResourceLoader * Loader = m_ResourceLoaders[Type];
	CRenderResource * Resource = Loader->GetResource(MeshID & RES_MASK);
	CMesh * Mesh = (CMesh *)Resource;
	Mesh->SetGeometryData(VBuffer, VSize, IBuffer, INum);
	return 0;
}

//int ResourceManager::CResourceManager::CreateMesh(CMesh * Mesh)
//{
//	//if(!m_MeshCreatingBatch)
//	//{
//	//	m_MeshCreatingBatch = AllocMeshCreationBatch();
//	//	m_MeshCreatingBatch->CMDNum = 0;
//	//	m_MeshCreatingBatch->Permanent = true;
//	//}
//	//else
//	//{
//	//	if(m_MeshCreatingBatch->CMDNum  == 32)
//	//	{
//	//		FlushCreationCMD();
//	//		m_MeshCreatingBatch = AllocMeshCreationBatch();
//	//		m_MeshCreatingBatch->CMDNum = 0;
//	//		m_MeshCreatingBatch->Permanent = true;
//	//	}	
//	//}
//	//// Find some Free Mesh 
//	//m_MeshCreatingBatch->CMD[m_MeshCreatingBatch->CMDNum].CreateId = FindFreeMesh(Mesh);
//	//m_MeshCreatingBatch->CMD[m_MeshCreatingBatch->CMDNum].Id = Mesh->m_ResourceID;
//	//m_MeshCreatingBatch->CMD[m_MeshCreatingBatch->CMDNum].VBuffer = Mesh->m_Vertex;
//	//m_MeshCreatingBatch->CMD[m_MeshCreatingBatch->CMDNum].VertexType = Mesh->m_VertexType;
//	//m_MeshCreatingBatch->CMD[m_MeshCreatingBatch->CMDNum].VSize = Mesh->m_VSize;
//	//m_MeshCreatingBatch->CMD[m_MeshCreatingBatch->CMDNum].IBuffer = Mesh->m_Index;
//	//m_MeshCreatingBatch->CMD[m_MeshCreatingBatch->CMDNum].INum = Mesh->m_IndexNum;
//	//m_MeshCreatingBatch->CMDNum++;
//	return 0;
//}

int ResourceManager::CResourceManager::CreateResource(int Type, CRenderResource * Resource)
{
	CreationCommand * CMD;
	int ID = m_ResourceCMDContainer.AllocResource(&CMD);
	CMD->CmdId = ID;
	CMD->m_Resource = Resource;
	CResourceLoader * Loader = m_ResourceLoaders[Type];
	CMD->CreateId = Loader->GetFreeReourceID(Resource);
	// submit to render
	CMD->Submit();
	return 0;
}

int ResourceManager::CResourceManager::CompleteResource(CreationCommand * CMD)
{
	m_CompleteQueue.AddCreationBatch(CMD);
	return 0;
}

int ResourceManager::CResourceManager::RegisterBone(char *File,char * ModelName,int MeshIndex)
{
	CBoneInfo * Bone;
	int CmdID;
	int ID = m_BoneInfoPool.AllocResource(&Bone);
	Bone->m_ResourceID = ID;
	Bone->SetPermanent(true);
	Bone->Bind(File,ModelName,0);

	LoadingCMD * CMD;
	CmdID = m_LoadingCMDContainer.AllocResource(&CMD);
	CMD->ID = CmdID;
	CMD->Resource = Bone;
	AddLoadingCMD(CMD);
	/*Bone->Load();
	Bone->IntoVRAM();*/
	return ID;
}

int ResourceManager::CResourceManager::UsingTexture(int ResourceID)
{
	CTexture * Texture = m_TexturePool.GetResourceByID(ResourceID);
	if(Texture && Texture->m_Status == RES_USING)
		return Texture->m_ResourceIDVRAM;
	return -1;
}

int ResourceManager::CResourceManager::UsingMesh(int ResourceID)
{
	if (ResourceID == -1)
		return -1;
	int Type = ResourceID >> RES_TYPE_SHIFT;
	return m_ResourceLoaders[Type]->UseResource(ResourceID & RES_MASK);
}

int ResourceManager::CResourceManager::UsingBone(int ResourceID)
{
	if(ResourceID == -1)
	{
		return -1;
	}
	CBoneInfo * Bone = m_BoneInfoPool.GetResourceByID(ResourceID);
	if(Bone)
		return Bone->m_ResourceIDVRAM;
	return -1;
}

int ResourceManager::CResourceManager::GetBoneFrams(int ResourceID)
{
	CBoneInfo *  Bone = m_BoneInfoPool.GetResourceByID(ResourceID);
	if(Bone)
		return Bone->m_Frames;
	return 0;
}

int ResourceManager::CResourceManager::GetBoneNum(int ResourceID)
{
	CBoneInfo *  Bone = m_BoneInfoPool.GetResourceByID(ResourceID);
	if(Bone)
		return Bone->m_BoneNum;
	return 0;
}

Matrix4x4 * ResourceManager::CResourceManager::GetBoneData(int ResourceID)
{
	CBoneInfo *  Bone = m_BoneInfoPool.GetResourceByID(ResourceID);
	if(Bone)
		return Bone->m_BoneData;
	return NULL;
}

int ResourceManager::CResourceManager::LoadingThread(CResourceManager * ResManager)
{
	LoadingCMD * CMD;
	CResourceLoader * Loader = NULL;
	CRenderResource * Resource = NULL;
	while(1)
	{
		CMD = ResManager->GetLoadingCMD();
		// process CMD
		if(CMD)
		{
			Loader = ResManager->GetResourceLoader(CMD->Type);
			Resource = CMD->Resource;
			if(CMD && !CMD->UnLoad)
			{
				Loader->LoadResource(Resource->m_ResourceID);
			}
			if(CMD && CMD->UnLoad)
			{
				Loader->UnLoadResource(Resource->m_ResourceID);
			}
			ResManager->AddLoadingOver(CMD);
		}
	}
	return 0;
}

LoadingCMD * ResourceManager::CResourceManager::GetLoadingCMD(void)
{
	LoadingCMD * CMD = NULL;
	WaitForSingleObject(m_LoadingEvent,-1);
	WaitForSingleObject(m_LoadingLock,-1);
	CMD = m_LoadingQueue.RemoveBatch();
	if(!CMD)
	{
		ResetEvent(m_LoadingEvent);
	}
	ReleaseMutex(m_LoadingLock);
	return CMD;
}

int ResourceManager::CResourceManager::AddLoadingCMD(LoadingCMD * CMD)
{
	WaitForSingleObject(m_LoadingLock,-1);
	if(m_LoadingQueue.IsEmpty())
	{
		SetEvent(m_LoadingEvent);
	}
	m_LoadingQueue.AddCreationBatch(CMD);
	ReleaseMutex(m_LoadingLock);
	return 0;
}

int ResourceManager::CResourceManager::AddLoadingOver(LoadingCMD * CMD)
{
	m_LoadingOverQueue.AddCreationBatch(CMD);
	return 0;
}

LoadingCMD * ResourceManager::CResourceManager::GetLoadingOver(void)
{
	LoadingCMD * CMD = 0;
	CMD = m_LoadingOverQueue.RemoveBatch();
	return CMD;
}

// // Resource load over create it now
int ResourceManager::CResourceManager::ProcessCreation(void)
{
	LoadingCMD * CMD = 0;
	CRenderResource * Resource;
	while(CMD = GetLoadingOver())
	{
		int Type = CMD->Type;
		Resource = CMD->Resource;
		if(!CMD->UnLoad)
		{  // loading cmd
			if(!Resource->m_RAMOnly)
			{
				//need vram
				CreateResource(Type, Resource);
			}
			if(Resource->m_RAMOnly)
			{
				//not need vram,ram only   only  out  delay and using status
				if(Resource->m_Status == RES_DELAY_FREE)
				{
					UnLoadResource(Type, Resource);
					Resource->m_Status = RES_OUT;
				}
				else
				{
					Resource->m_Status = RES_USING;
				}
			}
		}
		m_LoadingCMDContainer.FreeResource(CMD->ID);
	}
	return 0;
}

// return free geomety id in VRAM
int ResourceManager::CResourceManager::FindFreeMesh(CMesh * LoadingMesh)
{
	//FreeResource * FreeMesh = NULL;
	//CMesh * Mesh;
	//while(FreeMesh = m_FreeGeometry.GetNextResource(FreeMesh))
	//{
	//	Mesh = m_MeshPool.GetResourceByID(FreeMesh->ResID);
	//	if(Mesh->m_Status != RES_FREE)
	//	{
	//		return -1;
	//	}
	//	if(Mesh->m_VSizeVRAM >= LoadingMesh->m_VSize && Mesh->m_IndexNumVRAM >= LoadingMesh->m_IndexNum)
	//	{
	//		LoadingMesh->m_VSizeVRAM = Mesh->m_VSizeVRAM;
	//		LoadingMesh->m_IndexNumVRAM = Mesh->m_IndexNumVRAM;
	//		Mesh->m_Status = RES_OUT;
	//		UnLoadResource(Mesh);
	//		m_FreeGeometry.FreeResource(FreeMesh->ID);
	//		return Mesh->m_ResourceIDVRAM;
	//	}
	//}
	//// can't find
	//LoadingMesh->m_VSizeVRAM = LoadingMesh->m_VSize;
	//LoadingMesh->m_IndexNumVRAM = LoadingMesh->m_IndexNum;
	return -1;
}

int ResourceManager::CResourceManager:: FindFreeTexture(CTexture * LoadingTexture)
{
	//FreeResource * FreeTexture = NULL;
	//CTexture * Texture;
	//while(FreeTexture = m_FreeTexture[LoadingTexture->m_FreeIndex].GetNextResource(FreeTexture))
	//{
	//	Texture = m_TexturePool.GetResourceByID(FreeTexture->ResID);
	//	if(Texture->m_Status != RES_FREE)
	//	{
	//		return -1;
	//	}
	//	if(Texture->m_Scale = LoadingTexture->m_Scale)
	//	{
	//		Texture->m_Status = RES_OUT;
	//		UnLoadResource(Texture);
	//		m_FreeTexture[LoadingTexture->m_FreeIndex].FreeResource(FreeTexture->ID);
	//		return Texture->m_ResourceIDVRAM;
	//	}
	//}
	////can't find
	return -1;
}

int ResourceManager::CResourceManager::LoadMesh(int ID)
{
	//CMesh * Mesh = m_MeshPool.GetResourceByID(ID);
	//if(Mesh->m_Status == RES_FREE)
	//{
	//	// do not need loading
	//	Mesh->m_Status = RES_USING;
	//	RemoveFromFreeMesh(Mesh);
	//	return 0;
	//}
	//else
	//{
	//	if(Mesh->m_Status == RES_OUT)
	//	{
	//		// we need to load this mesh
	//		Mesh->m_Status = RES_LOADING;
	//		LoadResource(Mesh);
	//	}
	//}
	return 0;
}

int ResourceManager::CResourceManager::UnLoadMesh(int ID)
{
	//CMesh * Mesh = m_MeshPool.GetResourceByID(ID);
	//if(Mesh->m_Status == RES_LOADING)
	//{
	//	Mesh->m_Status = RES_DELAY_FREE;
	//	return -1;
	//}
	//if(Mesh->m_RAMOnly)
	//{
	//	if(Mesh->m_Status == RES_USING)
	//	{
	//		UnLoadResource(Mesh);
	//		Mesh->m_Status = RES_OUT;
	//	}
	//}
	//else
	//{
	//	if(Mesh->m_Status != RES_USING)
	//	{
	//		return -1;
	//	}
	//	ToFreeMesh(Mesh);
	//}
	return 0;
}

int ResourceManager::CResourceManager::LoadStreamTextue(int ID)
{
	////CTexture * Texture = m_TexturePool.GetResourceByID(ID);
	////if(Texture->m_Status == RES_FREE)
	////{
	////	// do not need loading
	////	Texture->m_Status = RES_USING;
	////	RemoveFromFreeTexture(Texture);
	////	return 0;
	////}
	////else
	////{
	////	if(Texture->m_Status == RES_OUT)
	////	{
	////		// we need to load this mesh
	////		Texture->m_Status = RES_LOADING;
	////		LoadResource(Texture);
	////	}
	//}
	return 0;
}

int ResourceManager::CResourceManager::UnLoadStreamTexture(int ID)
{
	return 0;
}

int ResourceManager::CResourceManager::ToFreeMesh(CMesh * Mesh)
{
	int ID;
	FreeResource * Res;
	Mesh->m_Status = RES_FREE;
	ID = m_FreeGeometry.AllocResource(&Res);
	Res->ID = ID;
	Res->ResID = Mesh->m_ResourceID;
	Mesh->m_FreeID = ID;
	return 0;
}

int ResourceManager::CResourceManager::ToFreeTexture(CTexture * Texture)
{
	int ID;
	FreeResource * Res;
	Texture->m_Status = RES_FREE;
	ID = m_FreeTexture[Texture->m_FreeIndex].AllocResource(&Res);
	Res->ID = ID;
	Res->ResID = Texture->m_ResourceID;
	Texture->m_FreeID = ID;
	return 0;
}

int ResourceManager::CResourceManager::RemoveFromFreeMesh(CMesh * Mesh)
{
	FreeResource * Res = NULL;
	m_FreeGeometry.FreeResource(Mesh->m_FreeID);
	return 0;
}

int ResourceManager::CResourceManager::RemoveFromFreeTexture(CTexture * Texture)
{
	FreeResource * Res = NULL;
	m_FreeTexture[Texture->m_FreeIndex].FreeResource(Texture->m_FreeID);
	return 0;
}

int ResourceManager::CResourceManager::LoadResource(int Type, CRenderResource * Resource)
{
	int CmdID;
	LoadingCMD * CMD;
	CmdID = m_LoadingCMDContainer.AllocResource(&CMD);
	CMD->ID = CmdID;
	CMD->Type = Type;
	CMD->Resource = Resource;
	CMD->UnLoad = false;
	AddLoadingCMD(CMD);
	return 0;
}

int ResourceManager::CResourceManager::UnLoadResource(int Type, CRenderResource * Resource)
{
	int CmdID;
	LoadingCMD * CMD;
	CmdID = m_LoadingCMDContainer.AllocResource(&CMD);
	CMD->ID = CmdID;
	CMD->Resource = Resource;
	CMD->Type = Type;
	CMD->UnLoad = true;
	AddLoadingCMD(CMD);
	return 0;
}

int ResourceManager::CResourceManager::GetMeshToEntity(CEntity * Entity)
{
	if(Entity && !Entity->m_ResourceUsing)
	{
		CResourceLoader * Loader = m_ResourceLoaders[MESH];
		CMesh * Mesh = (CMesh*)Loader->GetResource(Entity->GetMeshID() & RES_MASK);
		if(Mesh->m_Status == RES_USING)
		{
			Entity->m_INum = Mesh->m_IndexNum;
			Entity->m_VSize = Mesh->m_VSize;
			Entity->m_Vertex=Mesh->m_Vertex;
			Entity->m_Index=Mesh->m_Index;
			Entity->m_ResourceUsing = true;
		}
		else
		{
			Entity->m_Vertex = 0;
			Entity->m_Index = 0;
			Entity->m_INum = 0;
			Entity->m_VSize = 0;
		}
	}
	return 0;
}

ResourceManager::CResourceLoader::CResourceLoader()
{
	memset(m_FreeResource, 0, RESOURCE_SCALES * sizeof(CRenderResource*));
}

ResourceManager::CResourceLoader::~CResourceLoader()
{
}



CRenderResource * ResourceManager::CResourceLoader::AllocResource(ResourceDesc * Desc)
{
	return NULL;
}

CRenderResource * ResourceManager::CResourceLoader::MatchFreeResource(int Scale, CRenderResource * Resource)
{
	return NULL;
}

CRenderResource * ResourceManager::CResourceLoader::GetResource(int ID)
{
	return NULL;
}

int ResourceManager::CResourceLoader::GetFreeReourceID(CRenderResource * Resource)
{
	// get a free id of the space in VRAM for this new resource, or -1 if no free space
	CRenderResource * MatchedResource = MatchFreeResource(0, Resource);
	if (MatchedResource && MatchedResource->m_Status == RES_FREE)
		return MatchedResource->m_ResourceIDVRAM;
	else
		return -1;
}

int  ResourceManager::CResourceLoader::ToScale(int Scale, CRenderResource * Resource)
{

	if (!m_FreeResource[Scale])
	{
		m_FreeResource[Scale] = Resource;
		Resource->m_Next = Resource->m_Prev = Resource;
	}
	else
	{
		CRenderResource * Node = m_FreeResource[Scale];
		Resource->m_Next = Node;
		Resource->m_Prev = Node->m_Prev;
		Node->m_Prev->m_Next = Resource;
		Node->m_Prev = Resource;
	}
	return -1;
}

int  ResourceManager::CResourceLoader::RemoveFromFreeList(CRenderResource * Resource)
{
	return -1;
}
