#include "StdAfx.h"
#include "Include\RenderEngine.h"


CRenderEngine * CRenderEngine::m_ThisEngine = NULL;

CRenderEngine::CRenderEngine(void)
{
	Init();
	m_ThisEngine = this;
}

CRenderEngine::CRenderEngine(DWORD LogicThread)
{
	m_LogicThread = LogicThread;
	Init();
	m_ThisEngine = this;
}

CRenderEngine::~CRenderEngine(void)
{
}

int CRenderEngine::RenderScenes(float DeltTime)
{
	//process
	Process(DeltTime);
	//pre setups
	m_Render->ResetUpdateQueue();
	m_SceneManager->ClearShadowCastingLight();
	// first render none still node
	m_SceneManager->RenderNodes(-1,1);
	//render quead trees then,so the light Matrix is set
	CRect2D  ViewRect(m_Eye._x-100,m_Eye._z-100,m_Eye._x+100,m_Eye._z+100);
	CCullingFrustum Frustum = m_CurrentCamera->GetFrustum();
//	m_SceneManager->RenderQuadTrees(&ViewRect,1,0,1);
	m_SceneManager->RenderQuadTrees(&Frustum,1,0,1);
	
	// render shadowmaps with visible lights found in previous function calls, max 8 shadow maps
	m_SceneManager->RenderShadow();
	//update
	m_UpdateQueue = m_Render->UpDateBackQueue(m_UpdateQueue);
	return 0;
}

int CRenderEngine::Init(void)
{
	m_ModelLoader = new CModelLoader();
	m_ResourceManager = new CResourceManager();
	m_ResourceManager->RegisterResourceLoader(0, new CResourceLoader());
	m_ResourceManager->RegisterResourceLoader(MESH, m_ModelLoader);
	m_ResourceManager->RegisterResourceLoader(TEXTURE, new CTextureLoader());
	m_SceneManager = new CSceneManager();
	m_MaterialManager = new CMaterialManager();
	m_MaterialManager->Init();
	//Render
	m_Render = new CRender(m_LogicThread);
	m_UpdateQueue = m_Render->GetUpdateQueue();
	m_Render->BeginRender(true);
	//m_ActiveNode = NULL;   // set to null
	// Basic Entitis
	m_SceneManager->InitDefaultEntity();
	//culling
	m_CullingManager = new CCullingManager();
	return 0;
}



int CRenderEngine::SetGameCamera(CGameCamera * Camera)
{
	m_CurrentCamera = Camera;
	return 0;
}

CGameCamera * CRenderEngine::CreateGameCamera()
{
	CGameCamera * Camera = new CGameCamera();
	return Camera;
}

int CRenderEngine::Process(float DeltTime)
{
	m_ResourceManager->Update(DeltTime);
	// Set View Matrix
	Matrix4x4 * ViewMat = m_CurrentCamera->GetViewMatrix();
	Matrix4x4 * Projection = m_CurrentCamera->GetProjectionMatrix();
	m_Eye = m_CurrentCamera->GetCameraPos();
	m_Look = m_CurrentCamera->GetLook();
	m_Render->SetViewProjectionMatrix(*ViewMat,*Projection,m_Eye,m_Look);
	return 0;
}

Vector3 CRenderEngine::GetLook(void)
{
	return m_Look;
}

int CRenderEngine::InitRender(HWND hWnd, int Width, int Height)
{
	m_Render->InitRender(hWnd,Width,Height);
	return 0;
}

int CRenderEngine::RenderFrame()
{
	m_Render->RenderFrame();
	return 0;
}

int CRenderEngine::GetLodAABB(CCullingAABB * AABB)
{
	//  16  32  64 128
	Vector3 eye = GetEyePos();
	Vector3 Look = GetLook();
	CCullingPoint * Node = &AABB->Center();
	float AABBWidth = AABB->Length();
	AABBWidth = AABBWidth/2.0f;
	AABBWidth *= 1.414;
	float Distance = 
		(eye._x - Node->x) * (eye._x - Node->x) +
		(eye._y - Node->y) * (eye._y - Node->y) +
		(eye._z - Node->z) * (eye._z - Node->z);
	Distance = sqrtf(Distance) - AABBWidth; 
	if(Distance > 512)
		return 6;
	else if(Distance > 256)
		return 5;
	else if(Distance > 128)
		return 4;
	else if(Distance > 64)
		return 3;
	else if(Distance > 16)
		return 2;
	else if(Distance > 4)
		return 1;
	return 0;
}

int CRenderEngine::AABBInLodLevel(CCullingAABB * AABB, int Level)
{
	static float ranges[] = {32, 64, 128, 256, 512, 1024, 2048, 4096, 0, 0, 0, 0, 0, 0, 0, 0};
	if(Level == -1)
		return -1;
	float radius = ranges[Level];
	if(!radius)
		return -1;
	Vector3 eye = GetEyePos();
	CCullingPoint center(eye._x, eye._y, eye._z);
	CCullingSphere Sphere(center, radius);
	if(Sphere.Collision(AABB) == CCullingManager::INTERSECT)
	{
		return -1;
	}
	return 0;
}
