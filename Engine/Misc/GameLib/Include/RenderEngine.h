#ifndef __RENDER_ENGINE__
#define __RENDER_ENGINE__


#include "SceneObject.h"
#include "SceneLight.h"
#include "SceneSoftBody.h"
#include "SceneBasicGeometry.h"
#include "SceneSprite.h"
#include "SceneOccludee.h"
#include "Entity.h"

#include "QuadTree.h"

#include "ModelLoader.h"
#include "TextureLoader.h" 
#include "ResourceManager.h"
#include "SceneManager.h"
#include "MaterialManager.h"
#include "Render.h"
#include "RenderQueue.h"
#include "Terrain.h"

#include "ResourceContainer.h"

#include "GameCamera.h"
#include "FileLoader.h"

#include "CullingFrustum.h"

#include "point.h"

using SceneManager::CSceneObject;
using SceneManager::CSceneLight;
using SceneManager::CSceneBasicGeometry;
using ModelSystem::CModelLoader;
using Render::CRender;
using ResourceManager::CResourceManager;
using ResourceManager::CResourceContainer;
using Render::CRenderQueue;
using TerrainSystem::CTerrain;
using Loader::CFileLoader;
using namespace SceneManager;
using namespace MaterialSystem;
using namespace CullingSystem;
using namespace ResourceManager;




class CRenderEngine
{
private:
	// Managers
	CModelLoader *     m_ModelLoader;
	CRender      *     m_Render;
	CResourceManager * m_ResourceManager;
	CSceneManager *    m_SceneManager;
	CMaterialManager * m_MaterialManager;
	CCullingManager *  m_CullingManager;
	//Queue
	CRenderQueue *  m_UpdateQueue;
	//status
	Vector3         m_Eye;
	Vector3         m_Look;
	Matrix4x4       m_ViewMatrix;
	CGameCamera   * m_CurrentCamera;
	// curling stuff
	CRect2D        m_ViewRect;
	int            m_NumShadowLight;
	CRect2D        m_ShadowRect[MAX_SHADOWMAP];       // max shadowmaps, same as ShadowRect

	// game logic thread id
	DWORD           m_LogicThread;
private:
	int Init(void);
	int InitDefaultEntity(void);
public:
	CRenderEngine(void);
	CRenderEngine(DWORD LogicThread);
	~CRenderEngine(void);
	int RenderScenes(float DeltTime);
	CGameCamera * CreateGameCamera(void);
	int SetGameCamera(CGameCamera * Camera);

	Vector3 GetEyePos(void){return m_Eye;}
	Vector3 GetLook(void);
	int GetLodAABB(CCullingAABB * AABB);
	int AABBInLodLevel(CCullingAABB * AABB, int Level);
	int InitRender(HWND hWnd, int Width, int Height);
	int RenderFrame();
private:
	int Process(float DeltTime);
public:
	//static
	static CRenderEngine *   m_ThisEngine;
public:
	static inline CRenderEngine * GetRenderEngine(){return m_ThisEngine;}
	inline CSceneManager * GetSceneManager() {return m_SceneManager;}
	inline CMaterialManager * GetMaterialManager() {return m_MaterialManager;}
};

#endif
