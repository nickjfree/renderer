#include "Include\SceneLight.h"
#include "ResourceManager.h"


using SceneManager::CSceneLight;
using ResourceManager::CMesh;
using ResourceManager::CRenderResource;
using ResourceManager::CTexture;
using ResourceManager::CBoneInfo;
using ResourceManager::CResourceManager;

#define PI  (3.14159265358979323846)

CSceneLight::CSceneLight(void):m_ShadowCasting(false),m_LightIndex(-1)
{
	strcpy(m_Type,"SceneLight");
	m_LightData.shadowindex = -1;
	m_LightSimpleData.progress = -1;
	m_RenderParameter.LightIndex = -1;
	CreateScreenQuad();
}

CSceneLight::~CSceneLight(void)
{
}


int SceneManager::CSceneLight::Render(int LOD)
{
	CEntity * Entity = GetEntity(LOD);
	int MeshIDVRAM = -1,Texture = -1,Bone = -1;
	if(Entity)
	{
		CRender * Render = CRender::GetRender();
		Render->AddRenderable(Entity,m_Material[0],this);
	}
	CSceneNode::Render(LOD);
	return 0;
}

void CSceneLight::Refresh()
{
	Matrix4x4 TransMatrix;
	TransMatrix.Scale(m_Radius,m_Radius,m_Radius);
	m_LightSimpleData.Transform = TransMatrix * m_GlobalTrans;
	m_Position.x = m_GlobalTrans._41;
	m_Position.y = m_GlobalTrans._42;
	m_Position.z = m_GlobalTrans._43;
	m_Position.w = 1;
}

bool CSceneLight::IsShadowCastingLight()
{
	return m_ShadowCasting;
}

int SceneManager::CSceneLight::SetRadius(float radius)
{
	m_LightData.c3 = radius;                      // set c3 to radius
	m_Radius = radius;
	return 0;
}

int SceneManager::CSceneLight::SetColor(float r, float g, float b)
{
	m_LightData.c0 = r;
	m_LightData.c1 = g;
	m_LightData.c2 = b;
	m_Color.x = r;
	m_Color.y = g;
	m_Color.z = b;
	m_Color.w = 0;
	for(int i = 0;i < 4;i++)
	{
		m_LightParam.ScreenQuad[i].tangent._x = r;
		m_LightParam.ScreenQuad[i].tangent._y = g;
		m_LightParam.ScreenQuad[i].tangent._z = b;
	}
	return 0;
}

int SceneManager::CSceneLight::SetLightIndex(int Index)
{
	m_LightIndex=Index;
	return 0;
}

CRect2D& SceneManager::CSceneLight::GetBoundRect2D()
{
	float x = m_GlobalTrans._41;
	float z = m_GlobalTrans._43;
	m_BoundRect = CRect2D(x - m_Radius, z - m_Radius, x + m_Radius, z + m_Radius);
	return m_BoundRect;
}

void  * SceneManager::CSceneLight::GetRenderParameter()
{
	m_RenderParameter.LightIndex = m_LightIndex;
	return (void*)&m_RenderParameter;
}

int SceneManager::CSceneLight::SetShadowIndex(int index)
{
	m_LightData.shadowindex = index;
	return 0;
}

int SceneManager::CSceneLight::SetShadowLight(void)
{
	if(!m_LightData.shadowindex)
	{
		Matrix4x4 ViewMatrix;
		InverseMatrix(&ViewMatrix,&GetGlobalMatrix());
		//XMMATRIXInverse((XMMATRIX *)&ViewMatrix,NULL,(XMMATRIX *)&GetGlobalMatrix());
		//ViewMatrix = GetGlobalMatrix();
		ViewMatrix._11 = -1;
		ViewMatrix._41 = -ViewMatrix._41;
		//ViewMatrix._42 = -ViewMatrix._42;
		//ViewMatrix._43 = -ViewMatrix._43;
		//ViewMatrix.Identity();
		CRender::GetRender()->SetShadowMatrix(0,3.14 * 0.3,1,1000,ViewMatrix,m_LightData.Position);
	}
	return 0;
}

int SceneManager::CSceneLight::EnableShadow(void)
{
	m_ShadowCasting = true;
	return 0;
}

int SceneManager::CSceneLight::GetViewProjection(Matrix4x4 * View, Matrix4x4 * Projecion)
{
	InverseMatrix(&m_ViewMatrix,&GetGlobalMatrix());
	MatrixPerspectiveFovLH(&m_Projection, XM_PI * 0.25, 1.0f,1,m_Radius);
	m_ViewMatrix._11 = - m_ViewMatrix._11;
	m_ViewMatrix._21 = - m_ViewMatrix._21;
	m_ViewMatrix._31 = - m_ViewMatrix._31;
	m_ViewMatrix._41 = - m_ViewMatrix._41;
	*View = m_ViewMatrix;
	*Projecion = m_Projection;
	return 0;
}

int SceneManager::CSceneLight::CustructFrustum(CCullingFrustum * Frustum)
{
	Vector3 Right(m_ViewMatrix._11, m_ViewMatrix._21, m_ViewMatrix._31);
	Vector3 Up(m_ViewMatrix._12, m_ViewMatrix._22, m_ViewMatrix._32);
	Vector3 Look(m_ViewMatrix._13, m_ViewMatrix._23, m_ViewMatrix._33);
	Vector3 Position(m_GlobalTrans._41,m_GlobalTrans._42,m_GlobalTrans._43);
	CCullingManager::GetCullingManager()->ConstructFrustum(Frustum, Look, Right, Up, Position, 1, m_Radius, XM_PI * 0.25, 1.0f);
	return 0;
}

int SceneManager::CSceneLight::CreateScreenQuad(void)
{
	int m_Width = CRender::GetRender()->GetWidth();
	int m_Height = CRender::GetRender()->GetHeight();
	float farz = 1000.0f;
	float fary = farz * tan(XM_PI * 0.15f);
	float farx = fary *(float)m_Width/(float)m_Height;

	float neary = tan(XM_PI * 0.15f);
	float nearx = neary * (float)m_Width/(float)m_Height;
	//1
	m_LightParam.ScreenQuad[0].position = Vector3(-0,1,1);
	m_LightParam.ScreenQuad[0].u = 0;
	m_LightParam.ScreenQuad[0].v = 0;
	m_LightParam.ScreenQuad[0].normal = Vector3(-farx,fary,farz);
	m_LightParam.ScreenQuad[0].tangent = Vector3(0,0,0);

	//2
	m_LightParam.ScreenQuad[1].position = Vector3(1,1,1);
	m_LightParam.ScreenQuad[1].u = 1;
	m_LightParam.ScreenQuad[1].v = 0;
	m_LightParam.ScreenQuad[1].normal = Vector3(farx,fary,farz);
	m_LightParam.ScreenQuad[1].tangent = Vector3(0,0,0);
	//3
	m_LightParam.ScreenQuad[2].position = Vector3(-0,-0,1);
	m_LightParam.ScreenQuad[2].u = 0;
	m_LightParam.ScreenQuad[2].v = 1;
	m_LightParam.ScreenQuad[2].normal = Vector3(-farx,-fary,farz);
	m_LightParam.ScreenQuad[2].tangent = Vector3(0,0,0);
	//4
	m_LightParam.ScreenQuad[3].position = Vector3(1,-0,1);
	m_LightParam.ScreenQuad[3].u = 1;
	m_LightParam.ScreenQuad[3].v = 1;
	m_LightParam.ScreenQuad[3].normal = Vector3(farx,-fary,farz);
	m_LightParam.ScreenQuad[3].tangent = Vector3(0,0,0);
	return 0;
}

int SceneManager::CSceneLight::DrawLightPrePass(IMRender * Render,int * ContextKey, void * RenderParameter)
{
	Refresh();  // update required values
	// now the full screen quad alse check if with shadow map
	LightRenderParameter * Parameter = (LightRenderParameter *)RenderParameter;
	/*if(Parameter->LightIndex == -1 && m_ShadowCasting)
	{
		return -1;
	}*/
	int Context[CONTEXT_DIM];
	CEntity * Entity = GetEntity(0);
	memcpy(Context,ContextKey,CONTEXT_DIM * sizeof(int));
	Render->SetRenderContext(ContextKey);
	Render->SetWorldMatrix(&m_LightSimpleData.Transform);
//	Render->Render(0, Entity->m_Vertex, Entity->m_VSize, Entity->m_Index, Entity->m_INum);
	Render->RenderGeometry(Context[CONTEXT_GEOMETRY]);
	Render->Flush();
	if (Parameter->LightIndex != -1 )
	{
		Context[CONTEXT_PASS] = ContextKey[CONTEXT_PASS] + 2;
		Matrix4x4 LightView = Parameter->LightViewMatrix * Parameter->LightProjection;
		Render->SetLightViewMatrix(&LightView);
	}
	else
	{
		Context[CONTEXT_PASS] = ContextKey[CONTEXT_PASS] + 1;
	}
	// tricky, set contx_geometry to -1, or it will render using instancing.
	Context[CONTEXT_GEOMETRY] = -1;
	Render->SetRenderContext(Context);
	Render->SetLightParameter(&m_Position, &m_Color, m_Radius, 3, Parameter->LightIndex);
	Render->PostProcess();
	Render->Flush();
	return 0;
}
