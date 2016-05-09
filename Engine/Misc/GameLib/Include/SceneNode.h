#pragma once
#include "IMRender.h"
#include "Rect2D.h"
#include "MathLib.h"
#include "Entity.h"
#include "Material.h"
#include "CullingManager.h"

using SceneManager::CEntity;
using SceneManager::CRect2D;
using namespace MaterialSystem;

using namespace CullingSystem;

#define LIGHTPRE_PASS	0
#define NORMAL_PASS		1
#define SHADOW_PASS		2
#define REFLECTION_PASS 3



// LOD 
#define MAX_LODS  4


namespace SceneManager{

typedef struct InstanceInfo
{
	float       Progress;
	Matrix4x4   TransMatrix; 
}InstanceInfo;

class CSceneNode
{
protected:
	CEntity *   m_Entity[MAX_LODS];                       // gemometry or mesh
	CMaterial * m_Material[MAX_LODS];					    // render stats, everything need to be send to GPU when rendering. Skins(texture and material),shaders,contants,rasterazer,depth_stencil,samplers.
	CRect2D    m_BoundRect;
	// matrixt for render and progress
	InstanceInfo  m_InstanceInfo;
	// ids for render,ids in vram
	int         m_Texture;
	int         m_Mesh;
	int         m_Bone;
	int         m_NodeID;
	Matrix4x4   m_LocalTrans;
	Matrix4x4   m_GlobalTrans;
	Vector3     m_Origin;
	//refresh flag
	int        m_Refreshed;
	// culling proxy
	CCullingProxy * m_CullingProxy;
public:
	char m_Type[16];
	//Node list
	CSceneNode * m_NextNode;
	CSceneNode * m_PrevNode;
	CSceneNode * m_SubNode;
	CSceneNode * m_FatherNode;
	CSceneNode * m_RenderNext;       // node list for add to scene or quad tree
	CSceneNode * m_RenderPrev;
public:
	CSceneNode(void);
	virtual ~CSceneNode(void);
// funcs normal
public:
	virtual char * GetTypeString();
	virtual int Render(int LOD);
	virtual int RenderShadow(int LOD, int Index);
	virtual int RenderReflection();
	virtual int RenderLightPrePass();
	virtual CRect2D& GetBoundRect2D();
	virtual int      SetBoundRect2D(CRect2D& Rect);
	virtual int SetCullingProxy(CCullingProxy * Proxy) {m_CullingProxy = Proxy; return 0;};
	virtual CCullingProxy * GetCullingProxy() {return m_CullingProxy;};
	virtual bool     IsOccludee(){return false;}
	virtual void Refresh(){};
	virtual int SetOrigin(float x, float y, float z);
	int SetRotation(float a, float b, float c);
	InstanceInfo * GetInstanceInfo() {return &m_InstanceInfo;}
	// fucnions called by the CProcesser,this runs in the redering thread,so be carefull
	virtual int Draw(IMRender * Render,int Pass,int * ContextKey,void * RenderParameter);
	virtual bool IsShadowCastingLight() {return false;};
	virtual float GetRadius() {return 1.0f;};
	virtual void * GetRenderParameter() {return NULL;};
	virtual int SetLightIndex(int Index) {return 0;};
	virtual int GetViewProjection(Matrix4x4 * View, Matrix4x4 * Projecion){return -1;};
	virtual int CustructFrustum(CCullingFrustum * Frustum) {return -1;};
private:
	virtual int DrawLightPrePass(IMRender * Render,int * ContextKey, void * RenderParameter);
	virtual int DrawShadowPass(IMRender * Render ,int * ContextKey, void * RenderParameter);
	virtual int DrawNormalPass(IMRender * Render,int * ContextKey, void * RenderParameter);
	int UpdateGlobalTrans(void);
public:
	Matrix4x4& GetGlobalMatrix(void);
	int SetEntity(CEntity * Entity, int Lod);
	int GetEntityMeshID(void);
	CEntity * GetEntity(int Lod);
	int SetNodeID(int ID){m_NodeID = ID;return 0;}
	int GetNodeID(void){return m_NodeID;}
	int Attach(CSceneNode * Node);
	int SetLocalTransform(Matrix4x4& Transform);
	int SetScale(float x, float y, float z);
	int GetEntityID(void);
	CMaterial * GetMaterial() {return m_Material[0];}
	int SetMaterial(CMaterial * Material, int Lod) {m_Material[Lod] = Material;return 0;}
};

}