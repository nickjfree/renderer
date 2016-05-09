#ifndef __CULLING_SOLVER__
#define __CULLING_SOLVER__

#include "CullingPrimitive.h"
#include "CullingFrustum.h"


namespace CullingSystem {

class CCullingSolver
{
public:
	CCullingSolver(void);
	virtual ~CCullingSolver(void);
	int FrustumCollision(CCullingFrustum * Frustum, CCullingProxy * Proxy);
	int FrustumAARectTest(CCullingFrustum * Frustum, CCullingAARect * AARect);
	int FrustumAABBTest(CCullingFrustum * Frustum, CCullingAABB * AABB);
	int PlaneAARectTest(CCullingPlane * Plane, CCullingAARect * AARect);
	int PlaneAABBTest(CCullingPlane * Plane, CCullingAABB * AABB);
	int PlanePointTest(CCullingPlane * Plane, CCullingPoint * Point);
	int AARectCollision(CCullingAARect* AARect, CCullingProxy * Proxy);
	int SphereCollision(CCullingSphere * Sphere, CCullingProxy * Proxy);
	int SphereAABBTest(CCullingSphere * Sphere, CCullingAABB * AABB);
};


} // end namespace

#endif
