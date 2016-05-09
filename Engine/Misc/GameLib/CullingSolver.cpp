#include "Include\CullingSolver.h"
#include "CullingManager.h"


using namespace CullingSystem;

CCullingSolver::CCullingSolver(void)
{
}


CCullingSolver::~CCullingSolver(void)
{
}

int CCullingSolver::FrustumAARectTest(CCullingFrustum * Frustum, CCullingAARect * AARect)
{
	return 0;
}

int CCullingSolver::FrustumAABBTest(CCullingFrustum * Frustum, CCullingAABB * AABB)
{
	int CacheFlag = -1;
	int FinalFlag = CCullingManager::INSIDE;
	// check for cache first
	int SubIndex = AABB->GetCullingCache(Frustum);
	if(SubIndex != -1)
	{
		CacheFlag = PlaneAABBTest(&Frustum->m_Plane[SubIndex], AABB);
		if(CacheFlag == CCullingManager::OUTSIDE)
		{
			return CCullingManager::OUTSIDE;
		}
	}
	// cache not hit
	for(int i = 0; i < 6; i++)
	{
		int Flag = -1;
		if(CacheFlag != -1 && i == SubIndex)
		{
			Flag = CacheFlag;
		}
		else
		{
			Flag = PlaneAABBTest(&Frustum->m_Plane[i], AABB);
		}
		if(Flag == CCullingManager::OUTSIDE)
		{
			AABB->SetCullingCache(Frustum, i);
			FinalFlag = Flag;
			break;
		}
		else if(Flag == CCullingManager::INTERSECT)
		{
			FinalFlag = Flag;
		}
	}
	return FinalFlag;
}

int CCullingSolver::PlaneAARectTest(CCullingPlane * Plane, CCullingAARect * AARect)
{
	return 0;
}
int CCullingSolver::PlaneAABBTest(CCullingPlane * Plane, CCullingAABB * AABB)
{
	int MinFlag = PlanePointTest(Plane, &AABB->DirectionMin(Plane->normal));
	int MaxFlag = PlanePointTest(Plane, &AABB->DirectionMax(Plane->normal));
	if(MinFlag == MaxFlag)
	{
		return MinFlag;
	}
	else
	{
		return CCullingManager::INTERSECT;
	}
}

int CCullingSolver::PlanePointTest(CCullingPlane * Plane, CCullingPoint * Point)
{
	Vector3 point(Point->x,Point->y,Point->z);
	if (Plane->normal * point - Plane->d > 0)
	{
		return CCullingManager::OUTSIDE;
	}
	return CCullingManager::INSIDE;
}

int CCullingSolver::FrustumCollision(CCullingFrustum * Frustum, CCullingProxy * Proxy)
{
	switch (Proxy->m_Type)
	{
	case CCullingManager::AABB:
		return FrustumAABBTest(Frustum, (CCullingAABB*)Proxy);
	default:
		return CCullingManager::OUTSIDE;
	}
	return CCullingManager::OUTSIDE;
}

int CCullingSolver::AARectCollision(CCullingAARect* AARect, CCullingProxy * Proxy)
{
	return CCullingManager::OUTSIDE;
}

int CCullingSolver::SphereCollision(CCullingSphere * Sphere, CCullingProxy * Proxy)
{
	switch (Proxy->m_Type)
	{
	case CCullingManager::AABB:
		return SphereAABBTest(Sphere, (CCullingAABB *)Proxy);
	default:
		break;
	}
	return 0;
}


int CCullingSolver::SphereAABBTest(CCullingSphere * Sphere, CCullingAABB * AABB)
{
	float l = AABB->Length()/2;
	float w = AABB->Width()/2;
	float h = AABB->Height()/2;
	float distance = 0, xl = 0, yl = 0, zl = 0;
	float radius = Sphere->Radius();
	CCullingPoint sc = Sphere->Center();
	CCullingPoint bc = AABB->Center();
	xl = fabs(sc.x - bc.x);
	if(xl-l > radius)
		return CCullingManager::OUTSIDE;
	distance += (xl - l) * (xl - l);
	yl = fabs(sc.y - bc.y);
	if(yl-h > radius)
		return CCullingManager::OUTSIDE;
	distance += (yl - h) * (yl - h);
	zl = fabs(sc.z - bc.z);
	if(zl-w > radius)
		return CCullingManager::OUTSIDE;
	distance += (zl - w) * (zl - w);
	if(distance < radius * radius)
	{
		return CCullingManager::INTERSECT;
	}
	return CCullingManager::OUTSIDE;
}