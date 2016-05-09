#include "StdAfx.h"
#include "Include\QuadTree.h"
#include "SceneNode.h"
#include "RenderEngine.h"

using SceneManager::CQuadTree;
using SceneManager::CQuadTreeNode;
using SceneManager::CSceneNode;

CQuadTreeNode::CQuadTreeNode(void) :m_Occludee(0), m_Father(0), m_SenceNodes(0), m_NumTopLevelSenceNodes(0), m_OrphineNodes(0)
{
	memset(m_SubNode,0,4 * sizeof(LPVOID));
}

CQuadTreeNode::~CQuadTreeNode(void)
{
}

bool SceneManager::CQuadTreeNode::IsInside(CSceneNode * SenceNode)
{
	if(m_Bound.IsRect2DInside(SenceNode->GetBoundRect2D()))
	{
		return true;
	}
	return false;
}

int SceneManager::CQuadTreeNode::Create(CRect2D BoundRect,CQuadTree * Tree,int NodeLevel)
{
	m_Bound = BoundRect;
	// custruct culling AABB
	m_CullingAABB = CCullingAABB(CCullingPoint(BoundRect.UpLeft().m_x,0,BoundRect.UpLeft().m_z), CCullingPoint(BoundRect.LowRight().m_x,50,BoundRect.LowRight().m_z));
	m_IsLeaf = false;
	m_Level = NodeLevel;
	//if we are on leaf
	if(m_Level == 0)
	{
		m_IsLeaf = true;
		// add an OccludeeNode here
		m_Occludee = CRenderEngine::GetRenderEngine()->GetSceneManager()->CreateSceneOccludee();
		m_Occludee->SetMaterial(CRenderEngine::GetRenderEngine()->GetMaterialManager()->GetOccludeeMaterial(), 0);
		m_Occludee->SetOrigin(m_Bound.Center().m_x,50,m_Bound.Center().m_z);
		m_Occludee->SetScale(m_Bound.Width()/2,50,m_Bound.Height()/2);
		return 0;
	}
	//if any node for use
	if(Tree->IsNodePoolUsedUp())
	{
		//we are out of node now,
		//make it a leaf
		m_IsLeaf = true;
		return 0;
	}
	// Create subnodes
	CRect2D ULRect(m_Bound.UpLeft(),m_Bound.Center());
	CRect2D URRect(m_Bound.UpLeft().Middle(m_Bound.UpRight()),m_Bound.UpRight().Middle(m_Bound.LowRight()));
	CRect2D LLRect(m_Bound.UpLeft().Middle(m_Bound.LowLeft()),m_Bound.LowLeft().Middle(m_Bound.LowRight()));
	CRect2D LRRect(m_Bound.Center(),m_Bound.LowRight());

	CQuadTreeNode * SubNode;
	//Upleft
	SubNode = Tree->TreeNodeFromPool();
	SubNode->Create(ULRect,Tree,m_Level-1);
	m_SubNode[UL] = SubNode;
	//UpRight
	SubNode = Tree->TreeNodeFromPool();
	SubNode->Create(URRect,Tree,m_Level-1);
	m_SubNode[UR] = SubNode;
	//LowLeft
	SubNode = Tree->TreeNodeFromPool();
	SubNode->Create(LLRect,Tree,m_Level-1);
	m_SubNode[LL] = SubNode;
	//LowRight
	SubNode = Tree->TreeNodeFromPool();
	SubNode->Create(LRRect,Tree,m_Level-1);
	m_SubNode[LR] = SubNode;
	return 0;
}

int  SceneManager::CQuadTreeNode::Create(CCullingAARect& BoundRect,CQuadTree * Tree,int NodeLevel)
{
	m_CullingAARect = BoundRect;
	m_IsLeaf = false;
	m_Level = NodeLevel;
	//if we are on leaf
	if(m_Level == 0)
	{
		m_IsLeaf = true;
		// add an OccludeeNode here
		m_Occludee = CRenderEngine::GetRenderEngine()->GetSceneManager()->CreateSceneOccludee();
		m_Occludee->SetMaterial(CRenderEngine::GetRenderEngine()->GetMaterialManager()->GetOccludeeMaterial(), 0);
		m_Occludee->SetOrigin(BoundRect.Center().x,100,BoundRect.Center().z);
		m_Occludee->SetScale(BoundRect.Length()/2,100,BoundRect.Width()/2);
		return 0;
	}
	//if any node for use
	if(Tree->IsNodePoolUsedUp())
	{
		//we are out of node now,
		//make it a leaf
		m_IsLeaf = true;
		return 0;
	}
	// Create subnodes
	CCullingAARect ULRect(BoundRect.UpLeft(),BoundRect.Center());
	CCullingAARect URRect(BoundRect.UpCenter(), BoundRect.RightCenter());
	CCullingAARect LLRect(BoundRect.LeftCenter(),BoundRect.LowCenter());
	CCullingAARect LRRect(BoundRect.Center(),BoundRect.LowRight());

	CQuadTreeNode * SubNode;
	//Upleft
	SubNode = Tree->TreeNodeFromPool();
	SubNode->Create(ULRect,Tree,m_Level-1);
	m_SubNode[UL] = SubNode;
	//UpRight
	SubNode = Tree->TreeNodeFromPool();
	SubNode->Create(URRect,Tree,m_Level-1);
	m_SubNode[UR] = SubNode;
	//LowLeft
	SubNode = Tree->TreeNodeFromPool();
	SubNode->Create(LLRect,Tree,m_Level-1);
	m_SubNode[LL] = SubNode;
	//LowRight
	SubNode = Tree->TreeNodeFromPool();
	SubNode->Create(LRRect,Tree,m_Level-1);
	m_SubNode[LR] = SubNode;
	return 0;
}

int SceneManager::CQuadTreeNode::AddSenceNode(CSceneNode * Node, int Level)
{
	if(m_Level != Level)
	{
		if(!m_IsLeaf)
		{
			for(int i = 0;i < 4;i++)
			{
				if(m_SubNode[i]->IsInside(Node))
				{
					m_SubNode[i]->AddSenceNode(Node, Level);
					return 0;
				}
			}
			// hava to add to orphnes cause it is a LOD 0 object, add to first node
			if (!Level)
			{
				if (!m_OrphineNodes)
				{
					m_OrphineNodes = Node;
					Node->m_RenderNext = Node->m_RenderPrev = Node;
				}
				else
				{
					CSceneNode * InsertNode = m_OrphineNodes;
					// insert after InsertedNode
					Node->m_RenderNext = InsertNode->m_RenderNext;
					Node->m_RenderPrev = InsertNode;
					InsertNode->m_RenderNext = Node;
					Node->m_RenderNext->m_RenderPrev = Node;
				}
				return 0;
			}
		}
	}
	// subnodes can't take the node or we are on leafs,we keep the node
	if(!m_SenceNodes)
	{
		m_SenceNodes = Node;
		Node->m_RenderNext = Node->m_RenderPrev = Node;
	}
	else
	{
		CSceneNode * InsertNode = m_SenceNodes;
		// insert after InsertedNode
		Node->m_RenderNext = InsertNode->m_RenderNext;
		Node->m_RenderPrev = InsertNode;
		InsertNode->m_RenderNext = Node;
		Node->m_RenderNext->m_RenderPrev = Node;	
	}
	// add subnodes
	/*CSceneNode * SubNode = Node->m_SubNode;
	if(SubNode)
	{
		do
		{
			AddSenceNode(SubNode, Level);
			SubNode = SubNode->m_NextNode;
		}while(SubNode && SubNode != Node->m_SubNode);
	}*/
	m_NumTopLevelSenceNodes++;
	return 0;
}

int SceneManager::CQuadTreeNode::CurlingRender(CRect2D* Rect,int Num,int CrossIndex,int Shadow)
{
	if(!m_IsLeaf)
	{
		for(int i = 0;i < 4;i++)
		{
			for(int n = 0;n < Num;n++)
			{
				if(m_SubNode[i]->IsCrossRect2D(Rect[n]))
				{
					m_SubNode[i]->CurlingRender(Rect,Num,n,Shadow);
					//break;
				}
			}
		}
	}
	//leafs or have nodes
	if(m_NumTopLevelSenceNodes)
	{
		if(m_Occludee && !Shadow)
		{
			Vector3 eye = CRenderEngine::GetRenderEngine()->GetEyePos();
			CPoint2D point = CPoint2D(eye._x,eye._z);
			m_Occludee->Render(0);
			if(m_Occludee->IsOccludee() && !m_Bound.IsPointInside(point))
			{
				return -1;
			}
		}
		CSceneNode * Node = m_SenceNodes;
		while(1)
		{
			if(!Shadow)
			{
//				Node->RenderLightPrePass();
				CSceneManager::GetSceneManager()->PreRender(Node, -1);
				Node->Render(0);
				CSceneManager::GetSceneManager()->PostRender(Node, -1);
			}
			else
			{
				CSceneManager::GetSceneManager()->PreRender(Node, CrossIndex);
				Node->RenderShadow(0, CrossIndex);
				CSceneManager::GetSceneManager()->PostRender(Node, CrossIndex);
			}
			Node = Node->m_RenderNext;
			if(Node == m_SenceNodes)
				break;
		}
	}
	return 0;
}

int SceneManager::CQuadTreeNode::RenderOrphine(int Shadow, int CrossIndex)
{
	CSceneNode * Node = m_OrphineNodes;
	if (Node)
	{
		while (1)
		{
			if (!Shadow)
			{
				//				Node->RenderLightPrePass();
				CSceneManager::GetSceneManager()->PreRender(Node, -1);
				Node->Render(0);
				CSceneManager::GetSceneManager()->PostRender(Node, -1);
			}
			else
			{
				CSceneManager::GetSceneManager()->PreRender(Node, CrossIndex);
				Node->RenderShadow(0, CrossIndex);
				CSceneManager::GetSceneManager()->PostRender(Node, CrossIndex);
			}
			Node = Node->m_RenderNext;
			if (Node == m_OrphineNodes)
				break;
		}
	}
	return 0;
}

int SceneManager::CQuadTreeNode::RenderNodes(int Shadow, int CrossIndex)
{
	// if level of detail match current quadtree node
	if(m_Occludee && !Shadow)
	{
		Vector3 eye = CRenderEngine::GetRenderEngine()->GetEyePos();
		CPoint2D point = CPoint2D(eye._x,eye._z);
		m_Occludee->Render(0);
		if(m_Occludee->IsOccludee() && !m_Bound.IsPointInside(point))
		{
			return -1;
		}
	}
	CSceneNode * Node = m_SenceNodes;
	if(Node)
	{
		while(1)
		{
			if(!Shadow)
			{
	//				Node->RenderLightPrePass();
				CSceneManager::GetSceneManager()->PreRender(Node, -1);
				Node->Render(0);
				CSceneManager::GetSceneManager()->PostRender(Node, -1);
			}
			else
			{
				CSceneManager::GetSceneManager()->PreRender(Node, CrossIndex);
				Node->RenderShadow(0, CrossIndex);
				CSceneManager::GetSceneManager()->PostRender(Node, CrossIndex);
			}
			Node = Node->m_RenderNext;
			if(Node == m_SenceNodes)
				break;
		}
	}
	return 0;
}

int SceneManager::CQuadTreeNode::CurlingRender(CCullingFrustum * Frustum,int CrossIndex,int Shadow, int Flag)
{
//	int LodLevel = CRenderEngine::GetRenderEngine()->GetLodAABB(&m_CullingAABB);
	int InLOD = CRenderEngine::GetRenderEngine()->AABBInLodLevel(&m_CullingAABB, m_Level-1);
	if(!InLOD && m_NumTopLevelSenceNodes)
	{
		RenderNodes(Shadow,CrossIndex);
	}
	else if(!m_IsLeaf)
	{
		for(int i = 0;i < 4;i++)
		{
			if(Flag == CCullingManager::INSIDE)
			{
				m_SubNode[i]->CurlingRender(Frustum,CrossIndex,Shadow,Flag);
			}
			else
			{   
				int Result = m_SubNode[i]->TestFrustum(Frustum);
				if(Result != CCullingManager::OUTSIDE)
				{
					m_SubNode[i]->CurlingRender(Frustum,CrossIndex,Shadow, Result);
				}
			}
		}
	}
	else
	{
		RenderNodes(Shadow,CrossIndex);
	}
	// always render orphines
	RenderOrphine(Shadow, CrossIndex);
	return 0;
}

bool SceneManager::CQuadTreeNode::IsCrossRect2D(CRect2D Rect)
{
	return m_Bound.IsRect2DTouch(Rect);
}

int SceneManager::CQuadTreeNode::TestFrustum(CCullingFrustum * Frustum)
{
	return Frustum->Collision(&m_CullingAABB);
}

int SceneManager::CQuadTreeNode::TestViewRange(Vector3& Eye, float Radius)
{
	return 0;
}
