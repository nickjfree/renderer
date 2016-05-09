#ifndef __CULLING_FRUSTUM__
#define __CULLING_FRUSTUM__

#include "cullingproxy.h"
#include "CullingPrimitive.h"

namespace CullingSystem {

class CCullingFrustum :	public CCullingProxy
{
public:
	enum PLANES_INDEX
	{
		FRONT,
		BACK,
		LEFT,
		RIGHT,
		TOP,
		BOTTOM,
		TOTAL_PLANE
	};
	CCullingPlane  m_Plane[TOTAL_PLANE];
public:
	CCullingFrustum(void);
	virtual ~CCullingFrustum(void);
	virtual int Collision(CCullingProxy * Proxy);
};

} // end namespace


#endif
