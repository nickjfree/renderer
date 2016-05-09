#include "MaterialManager.h"


using namespace MaterialSystem;



CMaterialManager::CMaterialManager(void)
{
	m_Prefix = "materials/";
	m_Postfix = ".xml";
}


CMaterialManager::~CMaterialManager(void)
{
}

int CMaterialManager::Init()
{
	m_OccludeeMaterial = CreateMaterial("occludee");
	return 0;
}

CMaterial * MaterialSystem::CMaterialManager::CreateDefaultMaterial(void)
{
	CMaterial * Material = new CMaterial();
	return Material;
}

CMaterial * MaterialSystem::CMaterialManager::CreateMaterial(char * Name)
{

	CMaterial * Material;
	int ID = m_MaterialPool.AllocResource(&Material);
	Material->ID = ID;
	char Path[256];
	strcpy(Path,m_Prefix);
	strcat(Path,Name);
	strcat(Path,m_Postfix);
	m_XMLParser.Parse(Path);
	Material->Create(Name,&m_XMLParser);
	return Material;
}

CMaterial * MaterialSystem::CMaterialManager::GetOccludeeMaterial()
{
	return m_OccludeeMaterial;
}
