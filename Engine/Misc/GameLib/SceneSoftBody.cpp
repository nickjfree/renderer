#include "Include\SceneSoftBody.h"
#include "ResourceManager.h"

using ResourceManager::CResourceManager;
using SceneManager::CSceneSoftBody;

CSceneSoftBody::CSceneSoftBody(void)
{
	memset(&m_SoftData,0,sizeof(m_SoftData));
	m_SoftData.Translation.Identity();
}

CSceneSoftBody::~CSceneSoftBody(void)
{
	if(m_SoftData.trans_vertex)
	{
		delete m_SoftData.trans_vertex;
	}
}

void SceneManager::CSceneSoftBody::Refresh()
{
	if(m_Refreshed)
		return;
	memcpy(m_SoftData.SkinningMatrixRender,m_SoftData.SkinnkingMatrix,m_SoftData.Bones* sizeof(Matrix4x4));
	m_Refreshed = 1;
}

// transform the vertex with this matrices,position,normal,and tangent or texcoord
int SceneManager::CSceneSoftBody::SetTransform(Matrix4x4* TransMatrix,int Num)
{
	m_SoftData.SkinnkingMatrix = TransMatrix;
	m_SoftData.Bones = Num;
	//for(int i = 0;i < m_SoftData.VNum;i++)
	//{
	//	int index = m_SoftData.init_vertex[i].instance_id;
	//	// transform here
	//	//m_SoftData.vertex[i].position = m_SoftData.init_vertex[i].position * TransMatrix[index];
	//}
	return 0;
}

int SceneManager::CSceneSoftBody::Render(int LOD)
{
	CEntity * Entity = GetEntity(LOD);
	int MeshIDVRAM = -1,Texture = -1,Bone = -1;
	if(Entity)
	{
		CRender * Render = CRender::GetRender();
		Render->AddRenderable(Entity,m_Material[LOD],this);
	}
	CSceneNode::Render(LOD);
	return 0;
}

int SceneManager::CSceneSoftBody::RenderShadow(int LOD, int Index)
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

int SceneManager::CSceneSoftBody::RenderLightPrePass()
{
	return 0;
}

int SceneManager::CSceneSoftBody::DrawNormalPass(IMRender * Render,int * ContextKey,void * RenderParameter)
{
	Refresh();
	Render->SetSkinningMatrix(m_SoftData.SkinningMatrixRender,m_SoftData.Bones);
	Render->RenderGeometry(ContextKey[CONTEXT_GEOMETRY]);
	Render->Flush();
	m_Refreshed = 0;
	return 0;
}

int SceneManager::CSceneSoftBody::DrawLightPrePass(IMRender * Render,int * ContextKey,void * RenderParameter)
{
	Refresh();
//	Render->SetSkin(m_Texture);
	Render->SetSkinningMatrix(m_SoftData.SkinningMatrixRender,m_SoftData.Bones);
	Render->RenderGeometry(ContextKey[CONTEXT_GEOMETRY]);
	Render->Flush();
	return 0;
}

int SceneManager::CSceneSoftBody::DrawShadowPass(IMRender * Render,int * ContextKey,void * RenderParameter)
{
	Refresh();
	Render->SetSkinningMatrix(m_SoftData.SkinningMatrixRender,m_SoftData.Bones);
	Render->RenderGeometry(ContextKey[CONTEXT_GEOMETRY]);
	Render->Flush();
	return 0;
}
