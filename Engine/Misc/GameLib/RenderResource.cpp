#include "StdAfx.h"
#include "RenderResource.h"
#include "Resourcemanager.h"
#include "ModelLoader.h"
#include "TextureLoader.h"

using ResourceManager::CRenderResource;
using ResourceManager::CTexture;
using ResourceManager::CResourceManager;
using ResourceManager::CBoneInfo;

using ModelSystem::CModelLoader;

CRenderResource::CRenderResource(void):m_ResourceID(-1),m_ResourceIDRAM(-1),m_ResourceIDVRAM(-1)
{
}

CRenderResource::~CRenderResource(void)
{
}

int ResourceManager::CRenderResource::IntoVRAM(void)
{
	return 0;
}

int ResourceManager::CRenderResource::Load()
{
	return 0;
}

int ResourceManager::CRenderResource::SetPermanent(bool flag)
{
	m_UnPagedRAM = m_UnPagedVRAM = flag;
	return 0;
}

int ResourceManager::CRenderResource::ToFreeResource()
{
	return 0;
}

int ResourceManager::CRenderResource::CreateOnRender(IMRender * Render, int CreateId)
{
	// resource creation hadlers called by render thread
	return -1;
}

int ResourceManager::CRenderResource::Complete()
{

	return 0;
}


CTexture::CTexture(void):m_StreamType(CTexture::STREAM_NONE)
{
	memset(m_RAMDatas,0,sizeof(char*)*8);
}

CTexture::~CTexture(void)
{
}


int ResourceManager::CTexture::BindFile(wchar_t ** Filelist)
{
	memcpy(m_Files,Filelist,8 * sizeof(wchar_t *));
	return 0;
}

int ResourceManager::CTexture::BindStreamFile(char * File, char * Name, int Type)
{
	return 0;
}

int ResourceManager::CTexture::IntoVRAM(void)
{
//	CResourceManager::GetResourceManager()->CreateTexture(this);
	return 0;
}

int ResourceManager::CTexture::Init(void)
{
	CTexture::CTexture();
	return 0;
}

int CTexture::UnLoad()
{
	CTextureLoader::GetTextureLoader()->UnLoadTexture(this);
	return 0;
}

int CTexture::ToFreeResource(void)
{

	return 0;
}

int CTexture::CreateOnRender(IMRender * Render, int CreateId)
{
	if (CreateId == -1)
	{
		CreateId = Render->CreateTexture(m_Files, this->m_RAMDatas, m_Scale, m_StreamType);
	}
	if (m_StreamType)
	{
//		Render->GetDynamicTBuffer(CreateId, CopyBatch->CopyCmd[i].TBufferDes, CopyBatch->CopyCmd[i].RowPitch);
		void * Handle[8] = {};
		Render->GetTextureHandle(CreateId, Handle);
		if(m_Scale)
		{
			Render->UpdateTexture(Handle, m_RAMDatas, m_Scale, m_StreamType);
		}
	}
	return CreateId;
}

ResourceManager::CMesh::CMesh():m_Index(0),m_IndexNum(0),m_Vertex(0),m_VSize(0),m_VertexType(0),m_Loadable(1)
{
}

ResourceManager::CMesh::~CMesh()
{
}


int ResourceManager::CMesh::IntoVRAM(void)
{
//	CResourceManager::GetResourceManager()->CreateMesh(this);
	return 0;
}

int ResourceManager::CMesh::Bind(char * File,char * ModelName,int MeshIndex)
{
	strcpy(m_File,File);
	m_MeshIndex = MeshIndex;
	strcpy(m_ModelName,ModelName);
	return 0;
}

int ResourceManager::CMesh::SetGeometryData(void * VBuffer, int VSize, WORD* IBuffer, int INum)
{
	m_Vertex = VBuffer;
	m_VSize = VSize;
	m_Index = IBuffer;
	m_IndexNum = INum;
	m_Loadable = 0;
	return 0;
}

CBoneInfo::CBoneInfo(void)
{
}

CBoneInfo::~CBoneInfo(void)
{
}

int ResourceManager::CBoneInfo::IntoVRAM(void)
{
	if(m_UnPagedVRAM)
	{
//		CResourceManager::GetResourceManager()->CreateBone(this);
	}
	return 0;
}

int ResourceManager::CBoneInfo::Bind(char * File, char * ModelName, int MeshIndex)
{
	strcpy(m_File,File);
	strcpy(m_ModelName,ModelName);
	m_MeshIndex = MeshIndex;
	return 0;
}

int ResourceManager::CBoneInfo::Load(void)
{
	CModelLoader::GetModelLoader()->LoadBone(this);
	return 0;
}

int ResourceManager::CRenderResource::UnLoad(void)
{
	return 0;
}

int ResourceManager::CMesh::UnLoad(void)
{
	CModelLoader::GetModelLoader()->PutMappingBase(m_File);;
	return 0;
}

int ResourceManager::CMesh::ToFreeResource(void)
{
	CResourceManager::GetResourceManager()->ToFreeMesh(this);
	return 0;
}

int ResourceManager::CMesh::CreateOnRender(IMRender * Render, int CreateId)
{
	if (CreateId == -1)
	{
		CreateId = Render->CreateDynamicGeometry(m_Vertex, m_VSize,m_Index, m_IndexNum);
	}
	Render->ChangeDynamicBufferSize(CreateId, m_VSize, m_IndexNum);
	void * GeometryHandle = Render->GetDynamicGeometryHandle(CreateId);
	Render->UpdateDynamicGemometry(GeometryHandle, m_Vertex, m_VSize, m_Index, m_IndexNum);
	return CreateId;
}
