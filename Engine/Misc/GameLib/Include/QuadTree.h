#ifndef __QUADTREE__
#define __QUADTREE__


#include "Rect2D.h"
#include "SceneNode.h"
#include "CullingManager.h"

using SceneManager::CRect2D;
using namespace CullingSystem;

#define UL  0
#define UR  1
#define LL  2
#define LR  3

namespace SceneManager{          

class CQuadTree;

class CQuadTreeNode
{

private:
	CRect2D m_Bound;
	// culling proxy for culling, use one of these
	CCullingAARect m_CullingAARect;
	CCullingAABB   m_CullingAABB;
	CQuadTreeNode * m_SubNode[4];  
	CQuadTreeNode * m_Father;
	CSceneNode *    m_SenceNodes;   // nodes with same LOD as this tree nodes
	CSceneNode *    m_OrphineNodes;
	CSceneNode *    m_Occludee;
	//Level
	int             m_Level;
	bool            m_IsLeaf;
	int             m_NumTopLevelSenceNodes;
public:
	CQuadTreeNode(void);        
	~CQuadTreeNode(void);
	bool IsInside(CSceneNode * SenceNode);
	int Create(CRect2D BoundRect,CQuadTree * Tree,int NodeLevel);
	int Create(CCullingAARect& BoundRect,CQuadTree * Tree,int NodeLevel);
	int AddSenceNode(CSceneNode * Node, int Level = 0);
	int CurlingRender(CRect2D* Rect,int Num,int CrossIndex,int Shadow);
	int CurlingRender(CCullingFrustum * Frustum,int CrossIndex,int Shadow,int Flag);
	int RenderNodes(int Shadow, int CrossIndex);
	int RenderOrphine(int Shadow, int CrossIndex);
	bool IsCrossRect2D(CRect2D Rect);
	int TestFrustum(CCullingFrustum * Frustum);
	int TestViewRange(Vector3& Eye, float Radius);
};

class CQuadTree
{
private:
	CRect2D m_WorldRect;
	CQuadTreeNode * m_TreeNodePool;
	CQuadTreeNode * m_Root;
	//counters
	int  m_TotalNodes;
	int	 m_NodeUsed;
	int  m_LeafNum;
	int  m_Levels;
public:
	int  m_TreeID;                  // TreeID in container of QuadTree
	int  m_ActiveID;                // ID in the container which contains pointer of this tree
public:
	CQuadTree(void);

public:
	~CQuadTree(void);
	int Construct(CRect2D WorldBound, float Accuracy);
	int Construct(CCullingAARect& WorldBound, float Accuracy);
	CQuadTreeNode * TreeNodeFromPool(void);
	bool IsNodePoolUsedUp(void);
	int AddSenceNode(CSceneNode * Node, int Level = 0);
	int CurlingRender(CRect2D * ViewRects,int ViewNum,int Shadow);
	int CurlingRender(CCullingFrustum * Frustums,int FrustumsNum,int Shadow);
};

}// end namespace

#endif
