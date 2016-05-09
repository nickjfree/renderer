#include "Include\SceneSprite.h"
#include "Render.h"
#include "ResourceManager.h"

using namespace SceneManager;
using namespace Render;
using namespace ResourceManager;

WORD CSceneSprite::m_Index[6];

CSceneSprite::CSceneSprite(void)
{
	// do a test here,don't forget to remove this code later
	m_Vertex[0].position = Vector3(0,1,1);
	m_Vertex[0].u = 0;
	m_Vertex[0].v = 0;
	m_Vertex[1].position = Vector3(1,1,1);
	m_Vertex[1].u = 1;
	m_Vertex[1].v = 0;
	m_Vertex[2].position = Vector3(0,0,1);
	m_Vertex[2].u = 0;
	m_Vertex[2].v = 1;
	m_Vertex[3].position = Vector3(1,0,1);
	m_Vertex[3].u = 1;
	m_Vertex[3].v = 1;
	m_Index[0] = 0;
	m_Index[1] = 1;
	m_Index[2] = 2;
	m_Index[3] = 2;
	m_Index[4] = 1;
	m_Index[5] = 3;
}

CSceneSprite::~CSceneSprite(void)
{
}

int CSceneSprite::Render(int LOD)
{
	CRender * Render = CRender::GetRender();
	Render->AddRenderable(0,m_Material[LOD],this);
	return 0;
}

int SceneManager::CSceneSprite::DrawNormalPass(IMRender * Render,int * ContextKey,void * RenderParameter)
{
//	Refresh();
//	Render->Render(0,m_Vertex,4 * sizeof(vertex_dynamic_instancing),m_Index,6);
	Render->PostProcess();
	return 0;
}
