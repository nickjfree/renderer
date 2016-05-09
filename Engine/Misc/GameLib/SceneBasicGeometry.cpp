#include "Include\SceneBasicGeometry.h"
#include "Render.h"
#include "ResourceManager.h"

using namespace SceneManager;
using namespace Render;
using namespace ResourceManager;

CSceneBasicGeometry::CSceneBasicGeometry(void)
{
	m_ShapeAttrib.Reserved = -1;
}

CSceneBasicGeometry::~CSceneBasicGeometry(void)
{
}

int SceneManager::CSceneBasicGeometry::Render(int LOD)
{
	CEntity * Entity = GetEntity(LOD);
	int MeshIDVRAM = -1,Texture = -1,Bone = -1;
	if(Entity)
	{
		m_ShapeAttrib.Translation = GetGlobalMatrix();
		//m_InstanceInfo.Progress = -1;
		CRender * Render = CRender::GetRender();
		Render->AddRenderable(Entity,m_Material[LOD],this);
	}
	CSceneNode::Render(LOD);
	return 0;
}

void SceneManager::CSceneBasicGeometry::Refresh()
{
	m_InstanceInfo.TransMatrix = GetGlobalMatrix();
}

int SceneManager::CSceneBasicGeometry::DrawNormalPass(IMRender * Render,int * ContextKey,void * RenderParameter)
{
	Refresh();
	if(m_Mesh == -1)
			return -1;
	Render->SetWorldMatrix(&m_InstanceInfo.TransMatrix);
	Render->RenderGeometry(m_Mesh);
	Render->Flush();
	return 0;
}

CSceneBox::CSceneBox(void)
{
}

CSceneBox::~CSceneBox(void)
{
}
