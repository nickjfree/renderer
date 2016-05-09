#include "StdAfx.h"
#include "Include\ModelLoader.h"
#include "ResourceManager.h"

using ModelSystem::CModelLoader;
using ResourceManager::CResourceManager;


CModelLoader * CModelLoader::m_ThisLoader = NULL;

CModelLoader::CModelLoader(void)
{
	m_ThisLoader = this;
	memset(m_ModelMapping,0,MAX_PACK_MAPPING * sizeof(ModelFileMapping));
}

CModelLoader::~CModelLoader(void)
{
}

int ModelSystem::CModelLoader::LoadMesh(CMesh * Mesh)
{
	if (Mesh->m_Loadable)
	{
		void * Base = GetMappingBase(Mesh->m_File);
		h3d_header * Header = (h3d_header*)GetFileHeader(Base, Mesh->m_ModelName);
		h3d_mesh   * H3DMesh = GetH3DMesh(Header, Mesh->m_MeshIndex);
		Mesh->m_VSize = H3DMesh->VertexSize * H3DMesh->VertexNum;
		Mesh->m_Vertex = H3DMesh->OffsetVertex + (char*)Header;
		Mesh->m_IndexNum = H3DMesh->IndexNum;
		Mesh->m_Index = (WORD*)(H3DMesh->OffsetIndex + (char*)Header);
	}
	return 0;
}


h3d_mesh   *  ModelSystem::CModelLoader::GetH3DMesh(h3d_header * Header,int MeshIndex)
{
	h3d_mesh * Mesh = 0;
	if(Header->MeshNum < (MeshIndex+1))
	{
		return NULL;
	}
	return (h3d_mesh*)((char*)Header + sizeof(h3d_header) + MeshIndex * sizeof(h3d_mesh));
}

h3d_bone * ModelSystem::CModelLoader::GetH3DBone(h3d_header * header)
{
	h3d_bone * Bone = 0;	
	return (h3d_bone*)((char*)header + sizeof(h3d_header) + header->MeshNum * sizeof(h3d_mesh));
}

int ModelSystem::CModelLoader::LoadBone(CBoneInfo * Bone)
{
	void * Base = GetMappingBase(Bone->m_File);
	h3d_header * Header = (h3d_header*)GetFileHeader(Base,Bone->m_ModelName);
	h3d_bone *   H3DBone = GetH3DBone(Header);
	Bone->m_Frames = H3DBone->Frames;
	Bone->m_BoneNum = H3DBone->BoneNum;
	Bone->m_BoneData = (Matrix4x4*)((char*)Header + H3DBone->OffsetBone);
	return 0;
}


CRenderResource * ModelSystem::CModelLoader::AllocResource(ResourceDesc * Desc)
{	
	CMesh * Mesh;
	int ID = m_Pool.AllocResource(&Mesh);
	Mesh->m_ResourceID = ID;
	if (Desc)
	{
		Mesh->Bind(Desc->File, Desc->Name, Desc->SubIndex);
		Mesh->m_Desc = *Desc;
	}
	return Mesh;
}

int ModelSystem::CModelLoader::UseResource(int ID)
{
	CMesh * Mesh = m_Pool.GetResourceByID(ID);
	if (Mesh && Mesh->m_Status == RES_USING) {
		return Mesh->m_ResourceIDVRAM;
	}
	return -1;
}

int ModelSystem::CModelLoader::LoadResource(int ID)
{
	CMesh * Mesh = m_Pool.GetResourceByID(ID);
	if (Mesh)
	{
		LoadMesh(Mesh);
	}
	return 0;
}

int ModelSystem::CModelLoader::UnLoadResource(int ID)
{
	CMesh * Mesh = m_Pool.GetResourceByID(ID);
	if (Mesh)
	{
		if (Mesh->m_RAMOnly)
		{
			PutMappingBase(Mesh->m_File);
		}
		else
		{
			m_Pool.FreeResource(Mesh->m_ResourceID);
			ToScale(0, Mesh);
		}
	}
	return 0;
}

CRenderResource * ModelSystem::CModelLoader::GetResource(int ID)
{
	CMesh * Mesh = m_Pool.GetResourceByID(ID);
	return Mesh;
}
