#include "Include\CullingManager.h"
#include "mathlib.h"

using namespace CullingSystem;

CCullingManager * CCullingManager::m_this =NULL;

CCullingManager::CCullingManager(void)
{
	m_this = this;
}


CCullingManager::~CCullingManager(void)
{
}

int CCullingManager::ConstructFrustum(CCullingFrustum * Frustum, Vector3& Look, Vector3& Right, Vector3& Up, Vector3& Position, float Near, float Far, float Fov, float Aspect)
{
	float lmp = Look * Position;
	float ump = Up * Position;
	float rmp = Right * Position;
	float tanfy = tan(0.5*Fov);
	Vector3 LookOffset = Look * (-tanfy);
	Vector3 TopNormal = LookOffset + Up;
	Vector3 BottumNormal = LookOffset - Up; 
	Vector3 LeftNormal = LookOffset * Aspect - Right;
	Vector3 RightNormal = LookOffset * Aspect + Right;
	Vector3Normal(&TopNormal);
	Vector3Normal(&BottumNormal);
	Vector3Normal(&LeftNormal);
	Vector3Normal(&RightNormal);
	// construct six planes, in near,far,up,bottom,left,right order
	// near
	Frustum->m_Plane[CCullingFrustum::BACK].normal = Look * -1;
	Frustum->m_Plane[CCullingFrustum::BACK].d = -lmp - Near;
	// far
	Frustum->m_Plane[CCullingFrustum::FRONT].normal = Look;
	Frustum->m_Plane[CCullingFrustum::FRONT].d = lmp + Far;
	// top
	Frustum->m_Plane[CCullingFrustum::TOP].normal = TopNormal;
	Frustum->m_Plane[CCullingFrustum::TOP].d = TopNormal * Position;
	// bottom
	Frustum->m_Plane[CCullingFrustum::BOTTOM].normal = BottumNormal;
	Frustum->m_Plane[CCullingFrustum::BOTTOM].d = BottumNormal * Position;
	// left
	Frustum->m_Plane[CCullingFrustum::LEFT].normal = LeftNormal;
	Frustum->m_Plane[CCullingFrustum::LEFT].d = LeftNormal * Position;
	// right
	Frustum->m_Plane[CCullingFrustum::RIGHT].normal = RightNormal;
	Frustum->m_Plane[CCullingFrustum::RIGHT].d = RightNormal * Position;
	return 0;
}

CCullingAARect * CCullingManager::CreateCullingRect(CCullingPoint& min, CCullingPoint& max)
{
	CCullingAARect * Rect;
	int ID = m_AARectPool.AllocResource(&Rect);
	Rect->m_PoolID = ID;
	Rect->min = min;
	Rect->max = max;
	return Rect;
}

CCullingAABB * CCullingManager::CreateCullingAABB(CCullingPoint& min, CCullingPoint& max)
{
	CCullingAABB * AABB;
	int ID = m_AABBPool.AllocResource(&AABB);
	AABB->m_PoolID = ID;
	AABB->min = min;
	AABB->max = max;
	return AABB;
}

int CCullingManager::FrustumAARectTest(CCullingFrustum * Frustum, CCullingAARect * AARect)
{
	return m_CullingSolver->FrustumAARectTest(Frustum,AARect);
}

int CCullingManager::FrustumAABBTest(CCullingFrustum * Frustum, CCullingAABB * AABB)
{
	return m_CullingSolver->FrustumAABBTest(Frustum, AABB);
}

int CCullingManager::PlaneAARectTest(CCullingPlane * Plane, CCullingAARect * AARect)
{
	return m_CullingSolver->PlaneAARectTest(Plane, AARect);
}
int CCullingManager::PlaneAABBTest(CCullingPlane * Plane, CCullingAABB * AABB)
{
	return m_CullingSolver->PlaneAABBTest(Plane,AABB);
}

int CCullingManager::PlanePointTest(CCullingPlane * Plane, CCullingPoint * Point)
{
	return m_CullingSolver->PlanePointTest(Plane,Point);
}

int CCullingManager::FrustumCollision(CCullingFrustum * Frustum, CCullingProxy * Proxy)
{
	return m_CullingSolver->FrustumCollision(Frustum, Proxy);
}

int CCullingManager::AARectCollision(CCullingAARect* AARect, CCullingProxy * Proxy)
{
	return m_CullingSolver->AARectCollision(AARect,Proxy);
}

int CCullingManager::SphereCollision(CCullingSphere * Sphere, CCullingProxy * Proxy)
{
	return m_CullingSolver->SphereCollision(Sphere, Proxy);
}