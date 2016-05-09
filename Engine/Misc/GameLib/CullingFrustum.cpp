#include "Include\CullingFrustum.h"
#include "CullingManager.h"

using namespace CullingSystem;

CCullingFrustum::CCullingFrustum(void)
{
}


CCullingFrustum::~CCullingFrustum(void)
{
}

int CCullingFrustum::Collision(CCullingProxy * Proxy)
{
	return CCullingManager::GetCullingManager()->FrustumCollision(this,Proxy);
}
