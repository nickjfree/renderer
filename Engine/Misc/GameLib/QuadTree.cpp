#include "StdAfx.h"
#include "Include\QuadTree.h"
#include "SceneNode.h"

using SceneManager::CQuadTree;
using SceneManager::CQuadTreeNode;
using SceneManager::CSceneNode;

CQuadTree::CQuadTree(void)
{
}

CQuadTree::~CQuadTree(void)
{
}

int SceneManager::CQuadTree::Construct(CRect2D WorldBound, float Accuracy)
{
	int LeefNum = 0;
	int LeefRoot = 0;
	int NodeNum = 0;
	m_Levels = 0;
	//calc leef num
	LeefRoot = (int)(WorldBound.m_LR.m_x - WorldBound.m_UL.m_x)/(int)Accuracy;
	m_LeafNum = LeefNum = LeefRoot * LeefRoot;
	//calc Node num and level
	while(LeefNum)
	{
		m_Levels++;
		NodeNum += LeefNum;
		LeefNum = LeefNum/4;
	}
	m_TotalNodes = NodeNum;
	m_NodeUsed = 0;
	//alloc nodes
	m_TreeNodePool = new CQuadTreeNode[NodeNum];
	//link all node
	m_Root = TreeNodeFromPool();
	m_Root->Create(WorldBound,this,m_Levels-1);
	return 0;
}

int SceneManager::CQuadTree::Construct(CCullingAARect& WorldBound, float Accuracy)
{
	int LeefNum = 0;
	int LeefRoot = 0;
	int NodeNum = 0;
	m_Levels = 0;
	//calc leef num
	LeefRoot = (int)(WorldBound.Length())/(int)Accuracy;
	m_LeafNum = LeefNum = LeefRoot * LeefRoot;
	//calc Node num and level
	while(LeefNum)
	{
		m_Levels++;
		NodeNum += LeefNum;
		LeefNum = LeefNum/4;
	}
	m_TotalNodes = NodeNum;
	m_NodeUsed = 0;
	//alloc nodes
	m_TreeNodePool = new CQuadTreeNode[NodeNum];
	//link all node
	m_Root = TreeNodeFromPool();
	m_Root->Create(WorldBound,this,m_Levels-1);
	return 0;
}

CQuadTreeNode * SceneManager::CQuadTree::TreeNodeFromPool(void)
{
	if(m_NodeUsed == m_TotalNodes)
		return NULL;
	m_NodeUsed++;
	return &m_TreeNodePool[m_NodeUsed-1];
}

bool SceneManager::CQuadTree::IsNodePoolUsedUp(void)
{
	if(m_NodeUsed == m_TotalNodes)
		return true;
	return false;
}

int SceneManager::CQuadTree::AddSenceNode(CSceneNode * Node, int Level)
{
	m_Root->AddSenceNode(Node, Level);
	return 0;
}

int SceneManager::CQuadTree::CurlingRender(CRect2D* ViewRect,int ViewNum,int Shadow)
{
	m_Root->CurlingRender(ViewRect,ViewNum,0,Shadow);
	return 0;
}

int SceneManager::CQuadTree::CurlingRender(CCullingFrustum * Frustums,int FrustumsNum,int Shadow)
{
	for(int i = 0; i < FrustumsNum; i++)
	{
		m_Root->CurlingRender(&Frustums[i],i,Shadow,CCullingManager::INTERSECT);
	}
	return 0;
}
