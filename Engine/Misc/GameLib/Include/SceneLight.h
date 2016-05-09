#ifndef __SCENE_LIGHT__
#define __SCENE_LIGHT__

#include "SceneNode.h"
#include "Render.h"
#include "Entity.h"
#include "vertextype.h"
#include "Material.h"
#include "CullingManager.h"



using SceneManager::CSceneNode;
using SceneManager::CEntity;
using Render::CRender;
using namespace CullingSystem;

namespace SceneManager{


typedef struct _LightInstanceData
{
	float c0,c1,c2,c3;
	Vector3 Position;
	float shadowindex;
}LightInstanceData;

typedef struct _LightSimpleInstancing
{
	float     progress;
	Matrix4x4 Transform;
}LightSimpleInstancing;

typedef struct _LightParam
{
	vertex_dynamic_instancing ScreenQuad[4];
}LightParam;

typedef struct LightRenderParameter
{
	int LightIndex;
	Matrix4x4 LightViewMatrix;
	Matrix4x4 LightProjection;
}LightRenderParameter;


class CSceneLight : public CSceneNode 
{
protected:
	LightInstanceData  m_LightData;
	LightSimpleInstancing m_LightSimpleData;
	LightParam          m_LightParam;
	Vector4             m_Position;
	Vector4             m_Color;
	float               m_Radius;
	float               m_Intensity;
	bool                m_ShadowCasting;
	int                 m_LightIndex;
	LightRenderParameter m_RenderParameter;
	// Frustum parameter for light's view
	Matrix4x4           m_ViewMatrix;
	Matrix4x4           m_Projection;
private:
	int SetShadowLight(void);
public:
	CSceneLight(void);
	virtual ~CSceneLight(void);
	virtual int Render(int LOD);
	virtual int DrawLightPrePass(IMRender * Render,int * ContextKey,void * RenderParameter);
	int SetRadius(float radius);
	virtual float GetRadius() {return m_Radius;};
	void SetShadowCasting(bool flag) {m_ShadowCasting = flag;};
	int SetColor(float r, float g, float b);
	int SetShadowIndex(int index);
	virtual void Refresh();
	virtual bool IsShadowCastingLight();
	virtual CRect2D& GetBoundRect2D();
	int EnableShadow(void);
	virtual int SetLightIndex(int Index);
	void * GetRenderParameter();
	virtual int GetViewProjection(Matrix4x4 * View, Matrix4x4 * Projecion);
	virtual int CustructFrustum(CCullingFrustum * Frustum);

private:
	int CreateScreenQuad(void);
};

}// end namespace

#endif