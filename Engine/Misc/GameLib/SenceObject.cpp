#include "StdAfx.h"
#include "Include\SceneObject.h"
#include "ResourceManager.h"
#include <stdio.h>

using SceneManager::CSceneObject;
using ResourceManager::CResourceManager;
using Render::CRender;

CSceneObject::CSceneObject(void)
{
	strcpy(m_Type,"SceneObject");
	m_InstanceInfo.Progress = -1;
	m_InstanceInfo.TransMatrix.Identity();
}

CSceneObject::~CSceneObject(void)
{
}

void SceneManager::CSceneObject::Refresh()
{
	if(m_Refreshed)
		return;
	m_InstanceInfo.TransMatrix = GetGlobalMatrix();
	m_InstanceInfo.TransMatrix.Transpose();
	m_Refreshed = -1;
}

int SceneManager::CSceneObject::Render(int LOD)
{
	CEntity * Entity = GetEntity(LOD);
	if(Entity)
	{
		CRender * Render = CRender::GetRender();
		Render->AddRenderable(Entity,m_Material[LOD],this);
	}
	CSceneNode::Render(LOD);
	return 0;
}

int SceneManager::CSceneObject::RenderLightPrePass()
{
	return 0;
}

int SceneManager::CSceneObject::RenderShadow(int LOD, int Index)
{
	CEntity * Entity = GetEntity(LOD);
	if(Entity)
	{
		CRender * Render = CRender::GetRender();
		Render->AddRenderable(Entity,m_Material[LOD],this,Index);
	}
	CSceneNode::RenderShadow(LOD, Index);
	return 0;
}

int SceneManager::CSceneObject::SetProgress(float Progress)
{
	//m_InstanceInfo.Progress = 0;
	if(Progress > 1)
	{
		Progress = Progress - (int)Progress;
	}
	CEntity * Entity = GetEntity(0);
	if(Entity)
	{
		m_InstanceInfo.Progress = Entity->GetFrams() * Progress;
	}
	return 0;
}

int SceneManager::CSceneObject::DrawNormalPass(IMRender * Render,int * ContextKey,void * RenderParameter)
{
	Refresh();
	Render->SetSkin(m_Texture);
	Render->UseBoneData(m_Bone);
	CEntity * Entity = GetEntity(0);
	if (Entity && Entity->IsRamOnly())
	{
		Render->SetWorldMatrix(&m_InstanceInfo.TransMatrix);
		Render->Render(0, Entity->m_Vertex, Entity->m_VSize, Entity->m_Index, Entity->m_INum);
	}
	else
	{
		if(m_Mesh == -1)
			return -1;
		Render->SetWorldMatrix(&m_InstanceInfo.TransMatrix);
		Render->RenderGeometry(ContextKey[CONTEXT_GEOMETRY]);
		//Render->RenderInstance(0,&m_InstanceInfo,sizeof(InstanceInfo),m_Mesh);
	}
	m_Refreshed = 0;
	return 0;
}

int SceneManager::CSceneObject::DrawLightPrePass(IMRender * Render,int * ContextKey,void * RenderParameter)
{
	Refresh();
	DrawNormalPass(Render,ContextKey,RenderParameter);
	return 0;
}

int SceneManager::CSceneObject::DrawShadowPass(IMRender * Render,int * ContextKey,void * RenderParameter)
{
	Refresh();
	DrawNormalPass(Render,ContextKey,RenderParameter);
	return 0;
}
