#include "Include\SceneManager.h"

using namespace SceneManager;

#include <stdio.h>

CSceneManager * CSceneManager::m_ThisManager = NULL;

CSceneManager::CSceneManager(void):m_CurrentShadowCastingLight(0)
{
	m_ThisManager = this;
	m_ActiveNode = NULL;   // set to null
	for(int i = 0;i < MAX_SHADOWMAP;i++)
	{
		m_ShadowCastingLight[i] = NULL;
	}
}


CSceneManager::~CSceneManager(void)
{
}


CEntity * CSceneManager::CreateEntity(void)
{
	CEntity * Entity;
	int ID = m_EntityPool.AllocResource(&Entity);
	Entity->SetEntityID(ID);
	return Entity;
}

CSceneNode * CSceneManager::CreateSceneNode(void)
{
	CSceneNode * Node;
	int ID = m_SceneNodePool.AllocResource(&Node);
	Node->SetNodeID(ID);
	return Node;
}

CSceneObject * CSceneManager::CreateSceneObject(void)
{
	CSceneObject * Object;
	int ID = m_SceneObjectPool.AllocResource(&Object);
	Object->SetNodeID(ID);
	return Object;
}

int CSceneManager::AddToScene(CSceneNode * Node, bool still)
{
	int EntityID = -1;
	if(still)
	{
	}
	else
	{
		if(!m_ActiveNode)
		{
			m_ActiveNode = Node;
			m_ActiveNode->m_RenderNext = m_ActiveNode->m_RenderPrev = m_ActiveNode;
		}
		else
		{
			bool Inserted = false;
			CSceneNode * InsertNode = m_ActiveNode;
			//EntityID = Node->GetEntityID();
			//// the following lines are no use, try remove these lines someday. no need to sort, because the render queue will sort node by context_key
			//do
			//{
			//	if(InsertNode->GetEntityID() <= EntityID && InsertNode->m_RenderNext->GetEntityID() >= EntityID)
			//	{
			//		//insert here, after InsertNode
			//		break;
			//	}
			//	InsertNode = InsertNode->m_RenderNext;
			//}while(InsertNode && InsertNode->m_RenderNext != m_ActiveNode);
			// insert after InsertedNode
			Node->m_RenderNext = InsertNode->m_RenderNext;
			Node->m_RenderPrev = InsertNode;
			InsertNode->m_RenderNext = Node;
			Node->m_RenderNext->m_RenderPrev = Node;
			
		}
		/*CSceneNode * SubNode = Node->m_SubNode;
		if(SubNode)
		{
			do
			{
				AddToScene(SubNode,0);
				SubNode = SubNode->m_NextNode;
			}while(SubNode && SubNode != Node->m_SubNode);
		}*/
	}
	return 0;
}



int CSceneManager::CreateW3ETerrain(char * MapFile)
{
	CTerrain * Terrain = new CTerrain;
	Terrain->LoadW3E(MapFile);
	Terrain->SetTexture(-1);
	Terrain->Create();

	CQuadTree * Tree = CreateQuadTree();
	Terrain->ToSceneManager(Tree);
	ActiveQuadTree(Tree);
	return 0;
}

int CSceneManager::RenderScenes(float DeltTime)
{
	return 0;
}

CQuadTree * CSceneManager::CreateQuadTree()
{
	CQuadTree * Tree;
	int ID = m_QuadTreePool.AllocResource(&Tree);
	Tree->m_TreeID = ID;
	return Tree;
}

int CSceneManager::ActiveQuadTree(CQuadTree * Tree)
{
	CQuadTree ** QuadTreeHolder;
	int ID = m_ActiveQuadTreePool.AllocResource(&QuadTreeHolder);
	* QuadTreeHolder = Tree;
	Tree->m_ActiveID = ID;
	return 0;
}

CSceneLight * CSceneManager::CreateSceneLight(void)
{
	CSceneLight * Light;
	int ID = m_SceneLightPool.AllocResource(&Light);
	Light->SetNodeID(ID);
	return Light;
}

CSceneScreenLight * CSceneManager::CreateSceneScreenLight(void)
{
	CSceneScreenLight * Light;
	int ID = m_SceneScreenLightPool.AllocResource(&Light);
	Light->SetNodeID(ID);
	return Light;
}

int CSceneManager::RemoveFromScene(CSceneNode * Node)
{
	//remove father
	Node->m_RenderPrev->m_RenderNext = Node->m_RenderNext;
	Node->m_RenderNext->m_RenderPrev = Node->m_RenderPrev;
	if(m_ActiveNode == Node)
	{
		m_ActiveNode = Node->m_RenderNext;
		if(m_ActiveNode == Node)
			m_ActiveNode = NULL;
	}
	//remove children
	if(Node->m_SubNode)
	{
		CSceneNode * Sub = Node->m_SubNode;
		do{
			RemoveFromScene(Node);
			Sub = Sub->m_NextNode;
		}while(Sub && Sub != Node->m_SubNode);
	}
	return 0;
}

// Render object in QuadTrees
int CSceneManager::RenderQuadTrees(CRect2D * Rect,int Num,int Shadow,int FindLight)
{
	CQuadTree ** FirstTree,** Tree;
	Tree = FirstTree = m_ActiveQuadTreePool.GetNextResource(NULL);
	int a = 0;
	if(Tree)
	{
		do
		{
			(*Tree)->CurlingRender(Rect,Num,Shadow);
			a++;
			Tree = m_ActiveQuadTreePool.GetNextResource(Tree);
		}while(Tree && Tree != FirstTree);
	}
	return 0;
}

int CSceneManager::RenderQuadTrees(CCullingFrustum * Frustums,int Num,int Shadow, int FindLight)
{
	CQuadTree ** FirstTree,** Tree;
	Tree = FirstTree = m_ActiveQuadTreePool.GetNextResource(NULL);
	int a = 0;
	if(Tree)
	{
		do
		{
			(*Tree)->CurlingRender(Frustums,Num,Shadow);
			a++;
			Tree = m_ActiveQuadTreePool.GetNextResource(Tree);
		}while(Tree && Tree != FirstTree);
	}
	return 0;
}

// render object not in QuadTrees,BspTrees(dynamic nodes)
int CSceneManager::RenderNodes(int LightIndex,int FindLight)
{
	CSceneNode * Node = m_ActiveNode;
	if(Node)
	{
		do
		{
			if(LightIndex != -1)
			{
				PreRender(Node, LightIndex);
				Node->RenderShadow(0, LightIndex);  // Rendder this for LightIndex
				PostRender(Node, LightIndex);
			}
			else
			{
				PreRender(Node, -1);
				Node->Render(0);                // -1, Not shadow
				PostRender(Node,-1);
			}
			Node = Node->m_RenderNext;
		}while(Node && Node != m_ActiveNode);
	}
	return 0;
}

 int CSceneManager::AddShadowCastingLight(CSceneNode * LightNode)
 {
	 if(m_CurrentShadowCastingLight >= MAX_SHADOWMAP)
	 {
		 return -1;
	 }
	 m_ShadowCastingLight[m_CurrentShadowCastingLight] = LightNode;
	 LightNode->SetLightIndex(m_CurrentShadowCastingLight);
	 m_CurrentShadowCastingLight++;
	 return 0;
 }


 int CSceneManager::ClearShadowCastingLight()
 {
	 while(m_CurrentShadowCastingLight--)
	 {
		 m_ShadowCastingLight[m_CurrentShadowCastingLight]->SetLightIndex(-1);
	 }
	 m_CurrentShadowCastingLight = 0;
	 return 0;
 }

int CSceneManager::RenderShadow(void)
{
	// render shadow maps of current shadow casting lights
	CSceneNode * LightNode;
	CRender * Render = CRender::GetRender();
	CRect2D  LightRect[MAX_SHADOWMAP];
	CCullingFrustum LightFrustum[MAX_SHADOWMAP];
	for(int i = 0;i < m_CurrentShadowCastingLight;i++)
	{
		LightNode = m_ShadowCastingLight[i];
		float radius = LightNode->GetRadius();
		Matrix4x4 LightViewMatrix,LightProjection;
		LightNode->GetViewProjection(&LightViewMatrix,&LightProjection);
		Render->SetLightViewProjectionMatrix(i, LightViewMatrix, LightProjection);
//		LightNode->SeyLightIndex(i);
		// now render the nodes can be seen by the light
		// render node in dynamic scene
		RenderNodes(i,0);
		LightRect[i] = LightNode->GetBoundRect2D();
		LightNode->CustructFrustum(&LightFrustum[i]);
	}
	// render node in quad tree
//	RenderQuadTrees(LightRect, m_CurrentShadowCastingLight,1,0);
	RenderQuadTrees(LightFrustum,m_CurrentShadowCastingLight,1,0);
	return 0;
}


int CSceneManager::PreRender(CSceneNode * Node, int Shadow)
{
	if (Shadow == -1 && Node->IsShadowCastingLight())
	{
//		printf("add to shadow casting light %d",Node->GetNodeID());
		AddShadowCastingLight(Node);
	}
	return 0;
}


int CSceneManager::PostRender(CSceneNode * Node, int Shadow)
{
	if (Shadow != -1)
	{
		//printf("Render %s:(%d)",Node->GetTypeString(), Node->GetNodeID());
	}
	return 0;
}

CSceneSoftBody * CSceneManager::CreateSceneSoftBody(void)
{
	CSceneSoftBody * SoftBody = NULL;
	int ID = m_SceneSoftBodyPool.AllocResource(&SoftBody);
	SoftBody->SetNodeID(ID);
	return SoftBody;
}

CSceneBasicGeometry * CSceneManager::CreateBasicGeometry(void)
{
	CSceneBasicGeometry * Geometry = NULL;
	int ID = m_BasicGeometryPool.AllocResource(&Geometry);
	Geometry->SetNodeID(ID);
	//default basic geometry is a box
	Geometry->SetEntity(m_EntityBox, 0);
	return Geometry;
}

CSceneSprite * CSceneManager::CreateSprite(void)
{
	CSceneSprite * Sprite;
	int ID = m_SpritePool.AllocResource(&Sprite);
	Sprite->SetNodeID(ID);
	Sprite->SetEntity(NULL, 0);
	return Sprite;
}

CSceneOccludee * CSceneManager::CreateSceneOccludee(void)
{
	CSceneOccludee * Occludee;
	int ID = m_SceneOccludeePool.AllocResource(&Occludee);
	Occludee->SetNodeID(ID);
	Occludee->SetEntity(m_EntityBox, 0);
	return Occludee;
}

CSceneInstancing * CSceneManager::CreateSceneInstancing(void)
{
	CSceneInstancing * Instance;
	int ID = m_SceneInstancingPool.AllocResource(&Instance);
	Instance->SetNodeID(ID);
	return Instance;
}

CSceneHDR * CSceneManager::CreateSceneHDR(void)
{
	CSceneHDR * HDRNode;
	int ID = m_HDRPool.AllocResource(&HDRNode);
	HDRNode->SetNodeID(ID);
	HDRNode->SetEntity(NULL, 0);
	return HDRNode;
}

int CSceneManager::InitDefaultEntity(void)
{
	m_EntityBox = CreateEntity();
	m_EntityBox->Create("basic.pack","box",0,0);
	m_EntityBox->Load();
	return 0;
}
