#ifndef __CULLING_MANAGER__
#define __CULLING_MANAGER__

#include "MathLib.h"
#include "CullingPrimitive.h"
#include "CullingFrustum.h"
#include "CullingSolver.h"
#include "ResourceContainer.h"

using ResourceManager::CResourceContainer;

namespace CullingSystem {

class CCullingManager
{
private:
	//this manager
	static CCullingManager * m_this;
	CCullingSolver * m_CullingSolver;
	// item pools
	CResourceContainer<CCullingPoint> m_PointPool;
	CResourceContainer<CCullingPlane> m_PlanePool;
	CResourceContainer<CCullingAARect> m_AARectPool;
	CResourceContainer<CCullingAABB>   m_AABBPool;
	CResourceContainer<CCullingFrustum> m_FrustumPool;

public:
	enum COLLISION_TYPE
	{
		OUTSIDE,
		INTERSECT,
		INSIDE,
	};
	enum PRIMITIVE_TYPE
	{
		AABB,
		AARECT,
		PLANE,
		POINT,
		FRUSTUM,
		PROXY,
	};

public:
	CCullingManager(void);
	virtual ~CCullingManager(void);
	static CCullingManager * GetCullingManager(){return m_this;};

public:
	CCullingAARect * CreateCullingRect(CCullingPoint& min, CCullingPoint& max);
	CCullingAABB * CreateCullingAABB(CCullingPoint& min, CCullingPoint& max);
	int ConstructFrustum(CCullingFrustum * Frustum,Vector3& Look, Vector3& Right, Vector3& Up, Vector3& Position, float Near, float Far, float Fov, float Aspect);
	int FrustumCollision(CCullingFrustum * Frustum, CCullingProxy * Proxy);
	int FrustumAARectTest(CCullingFrustum * Frustum, CCullingAARect * AARect);
	int FrustumAABBTest(CCullingFrustum * Frustum, CCullingAABB * AABB);
	int PlaneAARectTest(CCullingPlane * Plane, CCullingAARect * AARect);
	int PlaneAABBTest(CCullingPlane * Plane, CCullingAABB * AABB);
	int PlanePointTest(CCullingPlane * Plane, CCullingPoint * Point);
	int AARectCollision(CCullingAARect* AARect, CCullingProxy * Proxy);
	int SphereCollision(CCullingSphere * Sphere, CCullingProxy * Proxy);
};

}// end namespace

#endif
