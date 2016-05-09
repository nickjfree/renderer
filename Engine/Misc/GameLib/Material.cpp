#include "Include\Material.h"
#include "ResourceManager.h"
#include "memory.h"
#include "stdio.h"

using namespace MaterialSystem;
using namespace ResourceManager;

CMaterial::CMaterial(void):m_TextureID(-1),m_MaterialID(-1),m_TechniqueId(8),m_PassId(0),m_PreTechniqueId(8),m_PrePassId(0),m_HasNormal(0),
	m_ShadowTechniqueId(-1),m_ShadowPassId(-1),m_TerrainID(-1)
{
	memset(m_ContextKey,0,sizeof(int) * CONTEXT_DIM);
	m_ContextKey[0] = 8;
}


CMaterial::~CMaterial(void)
{
}


int CMaterial::Create(char * Name,CXMLParser * Parser)
{
	//get pre-shader major/minor code
	strcpy(m_Name,Name);
	m_ShadowTechniqueId = Parser->GetInt("shadow_shader/major");
	m_ShadowPassId = Parser->GetInt("shadow_shader/minor");
	m_PreTechniqueId = Parser->GetInt("pre_shader/major");
	m_PrePassId = Parser->GetInt("pre_shader/minor");
	m_TechniqueId = Parser->GetInt("shader/major");
	m_PassId = Parser->GetInt("shader/minor");
	//textrues
	wchar_t * Files[8] = {0,0,0,0,0,0,0,0};
	swprintf(m_diffuse,L"%S",Parser->GetString("texture/diffuse"));
	swprintf(m_normal,L"%S",Parser->GetString("texture/normal"));
	MakeFileList(m_diffuse,Files);
	if (wcslen(m_normal))
	{
		m_HasNormal = true;
	}
	MakeFileList(m_normal,&Files[4]);
	CResourceManager * ResManager = CResourceManager::GetResourceManager();
	ResourceDesc Desc = {};
	Desc.MemType = 0;
	Desc.TextureFileList = Files;
	m_TextureID = ResManager->RegisterResource(TEXTURE, &Desc);
	ResManager->LoadResource(m_TextureID);
	if(!strcmp(Name, "terrain"))
	{
		ResourceDesc Desc = {};
		Desc.Name = "heightmap";
		m_TerrainID = ResManager->RegisterResource(TEXTURE, &Desc);
		ResManager->LoadResource(m_TerrainID);
	}
	return 0;
}

int * CMaterial::GetContextKey(int MeshID)
{
	CResourceManager * ResManager = CResourceManager::GetResourceManager();
	memset(m_ContextKey,0,sizeof(int) * CONTEXT_DIM);
	m_ContextKey[CONTEXT_TECH] = m_TechniqueId;
	m_ContextKey[CONTEXT_PASS] = m_PassId;
	if(m_TerrainID != -1)
	{
		m_ContextKey[CONTEXT_MATERIAL] = ResManager->UsingResource(m_TerrainID);
	}
	else
	{
		m_ContextKey[CONTEXT_MATERIAL] = -1;
	}
	m_ContextKey[CONTEXT_TEXTURE] = ResManager->UsingResource(m_TextureID);
	m_ContextKey[CONTEXT_GEOMETRY] = ResManager->UsingResource(MeshID);
	return m_ContextKey;
}

int * CMaterial::GetPreContextKey(int MeshID)
{
	CResourceManager * ResManager = CResourceManager::GetResourceManager();
	memset(m_ContextKey,0,sizeof(int) * CONTEXT_DIM);
	m_ContextKey[CONTEXT_TECH] = m_PreTechniqueId;
	m_ContextKey[CONTEXT_PASS] = m_PrePassId;
	if(m_TerrainID != -1)
	{
		m_ContextKey[CONTEXT_MATERIAL] = ResManager->UsingResource(m_TerrainID);
	}
	else
	{
		m_ContextKey[CONTEXT_MATERIAL] = -1;
	}
	/*if(m_HasNormal)
	{*/
		m_ContextKey[CONTEXT_TEXTURE] = ResManager->UsingResource(m_TextureID);
	//}
	m_ContextKey[CONTEXT_GEOMETRY] = ResManager->UsingResource(MeshID);
	return m_ContextKey;
}

int * CMaterial::GetShadowContextKey(int MeshID)
{
	CResourceManager * ResManager = CResourceManager::GetResourceManager();
	memset(m_ContextKey,0,sizeof(int) * CONTEXT_DIM);
	m_ContextKey[CONTEXT_TECH] = m_ShadowTechniqueId;
	m_ContextKey[CONTEXT_PASS] = m_ShadowPassId;
	if(m_TerrainID != -1)
	{
		m_ContextKey[CONTEXT_MATERIAL] = ResManager->UsingResource(m_TerrainID);
	}
	else
	{
		m_ContextKey[CONTEXT_MATERIAL] = -1;
	}
	m_ContextKey[CONTEXT_TEXTURE] = ResManager->UsingResource(m_TextureID);
	m_ContextKey[CONTEXT_GEOMETRY] = ResManager->UsingResource(MeshID);
	return m_ContextKey;
}

int CMaterial::MakeFileList(wchar_t * String, wchar_t ** Files)
{ 
	int count = 0;
	Files[count] = String;
	for(int i = 0; i < wcslen(m_diffuse); i++)
	{
		if(m_diffuse[i] == L',')
		{
			count++;
			Files[count] = &String[i+1];
			//clear comma
			String[i] = NULL;
		}
	}
	return 0;
}
