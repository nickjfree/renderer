
#ifndef __SCENE_MANAGER__
#define __SCENE_MANAGER__

#include "ResourceContainer.h"
#include "SceneNode.h"
#include "SceneLight.h"
#include "SceneScreenLight.h"
#include "SceneInstancing.h"
#include "SceneObject.h"
#include "SceneOccludee.h"
#include "SceneSoftBody.h"
#include "SceneSprite.h"
#include "SceneHDR.h"
#include "QuadTree.h"

#include "Terrain.h"

using namespace ResourceManager;
using namespace TerrainSystem;

namespace SceneManager {

class CSceneManager
{
private:
	//this
	static CSceneManager *                      m_ThisManager;
	// Pool
	CResourceContainer<CEntity>					m_EntityPool;
	CResourceContainer<CSceneNode>				m_SceneNodePool;
	CResourceContainer<CSceneObject>			m_SceneObjectPool;
	CResourceContainer<CSceneInstancing>        m_SceneInstancingPool;
	CResourceContainer<CSceneLight>				m_SceneLightPool;
	CResourceContainer<CSceneScreenLight>       m_SceneScreenLightPool;
	CResourceContainer<CSceneSoftBody>			m_SceneSoftBodyPool;
	CResourceContainer<CSceneBasicGeometry>     m_BasicGeometryPool;
	CResourceContainer<CSceneSprite>            m_SpritePool;
	CResourceContainer<CSceneHDR>               m_HDRPool;
	CResourceContainer<CSceneOccludee>          m_SceneOccludeePool;
	CSceneNode *                      m_ActiveNode;                   // use these instead, all node that can be rendered, sorted by EntityID
	CResourceContainer<CQuadTree>     m_QuadTreePool;
	CResourceContainer<CQuadTree *>   m_ActiveQuadTreePool;
	//QuadTree
	CQuadTree                         m_QuadTree;       

	// Basic Entitis
	CEntity *      m_EntityBox;
	CEntity *      m_EntitySphere;

	CSceneNode *                    m_ShadowCastingLight[MAX_SHADOWMAP];
	int                              m_CurrentShadowCastingLight;

	// listener functions
	// not implement

public:
	enum LISTENER
	{
		PRE_RENDER,
		POST_RENDER,
	};

public:
	CSceneManager(void);
	virtual ~CSceneManager(void);

	//singleton
	static CSceneManager * GetSceneManager() {return m_ThisManager;}

public:
	CEntity * CreateEntity(void);
	CSceneNode * CreateSceneNode(void);
	CSceneObject * CreateSceneObject(void);
	CSceneLight * CreateSceneLight(void);
	CSceneScreenLight * CreateSceneScreenLight(void);
	CSceneSoftBody * CreateSceneSoftBody(void);
	CSceneBasicGeometry * CreateBasicGeometry(void);
	CSceneOccludee * CreateSceneOccludee(void);
	CSceneInstancing * CreateSceneInstancing(void);
	CSceneHDR * CreateSceneHDR(void);
	CSceneSprite * CreateSprite(void);
	int InitDefaultEntity(void);
	int AddToScene(CSceneNode * Node, bool still);
	int CreateW3ETerrain(char * MapFile);
	int RenderScenes(float DeltTime);
	CQuadTree * CreateQuadTree();
	int ActiveQuadTree(CQuadTree * Tree);
	int RemoveFromScene(CSceneNode * Node);
	Vector3 GetLook(void);
	// Render object in QuadTrees
	int RenderQuadTrees(CRect2D * Rect,int Num,int Shadow, int FindLight);
	int RenderQuadTrees(CCullingFrustum * Frustums,int Num,int Shadow, int FindLight);
	// render object not in QuadTrees,BspTrees
	int RenderNodes(int LightIndex, int FindLight);

	// Add shadow casting light
	int AddShadowCastingLight(CSceneNode * LightNode);
	int ClearShadowCastingLight();

	int PreRender(CSceneNode * Node, int Shadow);
	int PostRender(CSceneNode * Node, int Shadow);
public:
	int RenderShadow(void);
};

} // end namespace

#endif