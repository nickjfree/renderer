#include "StdAfx.h"
#include "Entity.h"
#include "ResourceManager.h"
#include <stdio.h>

using SceneManager::CEntity;
using namespace ResourceManager;

CEntity::CEntity(void):m_BoneID(-1),m_MeshID(-1),m_TextureID(-1),m_RefRenderQueue(0)
{
}

CEntity::~CEntity(void)
{
}

int CEntity::GetMeshID()
{
	return m_MeshID;
}

int SceneManager::CEntity::Create(char * File,char * ModelName, int ModelIndex,int MemoryType)
{
	//m_MeshID = CResourceManager::GetResourceManager()->RegisterStreamMesh(MemoryType,File,ModelName,ModelIndex);
	ResourceDesc Desc;
	Desc.File = File;
	Desc.MemType = 0;
	Desc.Name = ModelName;
	Desc.SubIndex = ModelIndex;
	m_MeshID = CResourceManager::GetResourceManager()->RegisterResource(MESH, &Desc);
	m_RamOnly = (bool)MemoryType; 
	return 0;
}

int SceneManager::CEntity::SetGeometryData(void * VBuffer, int VSize, WORD* IBuffer, int INum)
{
	CResourceManager * ResManager = CResourceManager::GetResourceManager();
	m_MeshID = ResManager->RegisterResource(MESH, NULL);
	ResManager->SetMeshData(m_MeshID, VBuffer, VSize, IBuffer, INum);
	return 0;
}

int SceneManager::CEntity::CreateBone(char * File, char * ModelName, int MeshIndex)
{
	m_BoneID = CResourceManager::GetResourceManager()->RegisterBone(File,ModelName,MeshIndex);
	return 0;
}

int SceneManager::CEntity::CreateTexture(wchar_t ** Files)
{
	m_TextureID = CResourceManager::GetResourceManager()->RegisterTexture(Files);
	return 0;
}

int SceneManager::CEntity::GetBoneID(void)
{
	return m_BoneID;
}

int SceneManager::CEntity::GetTextureID(void)
{
	return m_TextureID;
}

Matrix4x4 * SceneManager::CEntity::GetBoneData(void)
{
	if(m_BoneID != -1)
	{
		return CResourceManager::GetResourceManager()->GetBoneData(m_BoneID);
	}
	return NULL;
}

int SceneManager::CEntity::GetBoneNum(void)
{
	if(m_BoneID != -1)
	{
		return CResourceManager::GetResourceManager()->GetBoneNum(m_BoneID);
	}
	return 0;
}

int SceneManager::CEntity::GetFrams(void)
{
	if(m_BoneID != -1)
	{
		return CResourceManager::GetResourceManager()->GetBoneFrams(m_BoneID);
	}
	return 0;
}

int SceneManager::CEntity::Load(h3d_callback callback, void * Data)
{
	// Load every thing
	CResourceManager::GetResourceManager()->LoadResource(m_MeshID);
	CResourceManager::GetResourceManager()->RegisterCallback(m_MeshID, callback, Data);
	return 0;
}

int SceneManager::CEntity::UnLoad(void)
{
	/*if(m_RefRenderQueue < 0)
	{
		printf("Ref RenderQueue Error, CEntity  %d\n", m_RefRenderQueue);
		return -1;
	}*/
	m_ResourceUsing = false;
	CResourceManager::GetResourceManager()->UnLoadResource(m_MeshID);
	return 0;
}

int SceneManager::CEntity::RefMesh(void)
{
	CResourceManager::GetResourceManager();
	return 0;
}
