#include "StdAfx.h"
#include "SceneNode.h"


using SceneManager::CSceneNode;
using SceneManager::CRect2D;
using SceneManager::CEntity;

CSceneNode::CSceneNode(void) :m_NodeID(-1), m_Refreshed(0), m_CullingProxy(0)
{
	strcpy(m_Type,"SenceNode");
	m_FatherNode = m_SubNode = m_NextNode = m_PrevNode = NULL;
	m_LocalTrans.Identity();
	m_GlobalTrans = m_LocalTrans;
	memset(m_Entity, 0, sizeof(void*)* MAX_LODS);
	memset(m_Material, 0, sizeof(void*)* MAX_LODS);
}

CSceneNode::~CSceneNode(void)
{
}

char * CSceneNode::GetTypeString()
{
	return m_Type;
}

int CSceneNode::Render(int LOD)
{
	CSceneNode * Node;
	if (m_SubNode)
	{
		Node = m_SubNode;
		do
		{
			Node->Render(LOD);
			Node = Node->m_NextNode;
		} while (Node != m_SubNode);
	}
	return 0;
}

int CSceneNode::RenderLightPrePass()
{
	return 0;
}

int CSceneNode::RenderShadow(int LOD, int Index)
{
	CSceneNode * Node;
	if (m_SubNode)
	{
		Node = m_SubNode;
		do
		{
			Node->RenderShadow(LOD, Index);
			Node = Node->m_NextNode;
		} while (Node != m_SubNode);
	}
	return 0;
}

int CSceneNode::RenderReflection()
{
	return 0;
}

CRect2D& CSceneNode::GetBoundRect2D()
{
	return m_BoundRect;           // not implement,buggy 
}

int CSceneNode::SetBoundRect2D(CRect2D& Rect)
{
	m_BoundRect = Rect;
	if(m_SubNode)
	{
		CSceneNode * Node = m_SubNode;
		do
		{
			Node->SetBoundRect2D(Rect);
			Node = Node->m_NextNode;
		}while(Node && Node != m_SubNode);
	}
	return 0;
}

int SceneManager::CSceneNode::SetOrigin(float x, float y, float z)
{
	m_LocalTrans.Tansform(x,y,z);
	UpdateGlobalTrans();
	return 0;
}

int SceneManager::CSceneNode::SetRotation(float a, float b, float c)
{
	Matrix4x4 Rot;
	Rot.Identity();
	Rot.RotXYZ(a,b,c);
	m_LocalTrans = Rot * m_LocalTrans;
	UpdateGlobalTrans();
	return 0;
}

int SceneManager::CSceneNode::UpdateGlobalTrans(void)
{
	if(m_FatherNode)
	{
		m_GlobalTrans = m_LocalTrans * m_FatherNode->GetGlobalMatrix();
	}
	else
	{
		m_GlobalTrans = m_LocalTrans;
	}
	if(m_SubNode)
	{
		CSceneNode * Node = m_SubNode;
		while(1)
		{
			Node->UpdateGlobalTrans();
			Node = Node->m_NextNode;
			if(Node == m_SubNode)
				break;
		}
	}
	return 0;
}

Matrix4x4& SceneManager::CSceneNode::GetGlobalMatrix(void)
{
	return m_GlobalTrans;
}

int SceneManager::CSceneNode::SetEntity(CEntity * Entity, int Lod)
{
	m_Entity[Lod] = Entity;
	return 0;
}

int SceneManager::CSceneNode::GetEntityMeshID(void)
{
	if(m_Entity)
	{
		return m_Entity[0]->GetMeshID();
	}
	return -1;
}

CEntity * SceneManager::CSceneNode::GetEntity(int Lod)
{
	if(m_Entity)
	{
		return m_Entity[0];
	}
	return NULL;
}

int SceneManager::CSceneNode::Attach(CSceneNode * Node)
{
	//dettach from father
	if(Node->m_FatherNode)
	{
		Node->m_NextNode->m_PrevNode = Node->m_PrevNode;
		Node->m_PrevNode->m_NextNode = Node->m_NextNode;
	}
	if(m_SubNode)
	{
		Node->m_NextNode = m_SubNode;
		Node->m_PrevNode = m_SubNode->m_PrevNode;
		m_SubNode->m_PrevNode->m_NextNode = Node;
		m_SubNode->m_PrevNode = Node;
	}
	else
	{
		Node->m_NextNode = Node->m_PrevNode = Node;
		m_SubNode = Node;
	}
	Node->m_FatherNode = this;
	//Reset LocalMatrix
	/*
		MatrixX * FatherMatrix = ChildMatrix(Current child Matrix)
		MatrixX = ChildMatrix * Inv(FatherMatrix)
	*/
	Matrix4x4 ChildMat = Node->GetGlobalMatrix();
	Matrix4x4 FatherMat = GetGlobalMatrix();
	Matrix4x4 Inv,MatrixX;
	InverseMatrix(&Inv,&FatherMat);
	MatrixX = ChildMat * Inv;
	Node->SetLocalTransform(MatrixX);
	return 0;
}

int SceneManager::CSceneNode::SetLocalTransform(Matrix4x4& Transform)
{
	m_LocalTrans = Transform;
	UpdateGlobalTrans();
	return 0;
}

int SceneManager::CSceneNode::SetScale(float x, float y, float z)
{
	Matrix4x4 Scal;
	Scal.Scale(x,y,z);
	m_LocalTrans = Scal * m_LocalTrans;
	UpdateGlobalTrans();
	return 0;
}

int SceneManager::CSceneNode::GetEntityID()
{
	CEntity * Entity = GetEntity(0);
	if(Entity)
		return Entity->GetEntityID();
	else
		return -1;
}

int SceneManager::CSceneNode::Draw(IMRender * Render,int Pass,int * ContextKey,void * RenderParameter)
{
	// we don't do anything here,but in subclass,this will be complicated
	switch(Pass)
	{
	case LIGHTPRE_PASS:
		DrawLightPrePass(Render,ContextKey,RenderParameter);
		break;
	case NORMAL_PASS:
		DrawNormalPass(Render,ContextKey,RenderParameter);
		break;
	case SHADOW_PASS:
		DrawShadowPass(Render,ContextKey,RenderParameter);
		break;
	case REFLECTION_PASS:
		//not implemented
		break;
	default:
		//an error
		break;
	}
	return 0;
}

int SceneManager::CSceneNode::DrawLightPrePass(IMRender * Render,int * ContextKey,void * RenderParameter)
{
	// do nothing in base class
	return 0;
}

int SceneManager::CSceneNode::DrawShadowPass(IMRender * Render,int * ContextKey,void * RenderParameter)
{
	// do nothing in base class
	return 0;
}
int SceneManager::CSceneNode::DrawNormalPass(IMRender * Render,int * ContextKey,void * RenderParameter) 
{
	// do nothing in base class
	return 0;
}
