#include "Include\CullingProxy.h"
#include "CullingManager.h"


using namespace CullingSystem;

CCullingProxy::CCullingProxy(void):m_PoolID(-1),m_CacheCount(0)
{
	m_Type = CCullingManager::PROXY;
	memset(&m_CullingCache, 0, sizeof(CullingCache));
	memset(&m_CacheIndex, -1, MAX_CULLING_CACHE * sizeof(char));
}


CCullingProxy::~CCullingProxy(void)
{
}


int CCullingProxy::SetCullingCache(CCullingProxy * Proxy, int SubIndex)
{
	char index;
	if(GetCullingCache(Proxy) != -1)
	{
		return -1;
	}
	if(m_CacheCount < MAX_CULLING_CACHE)
	{
		index = m_CacheCount;
		m_CacheIndex[index] = index;
		m_CacheCount++;
	}
	else
	{
		index = m_CacheIndex[MAX_CULLING_CACHE-1];
	}
	m_CullingCache[index].Proxy = Proxy;
	m_CullingCache[index].SubIndex = SubIndex;
	return 0;
}

int CCullingProxy::GetCullingCache(CCullingProxy * Proxy)
{
	for(int i = 0; i < m_CacheCount;i++)
	{
		int index = m_CacheIndex[i];
		if(index == -1)
		{
			return -1;
		}
		if(m_CullingCache[index].Proxy == Proxy)
		{
			int tmp_index = m_CacheIndex[0];
			m_CacheIndex[0] = index;
			m_CacheIndex[i] = tmp_index;
			return m_CullingCache[index].SubIndex;
		}
	}
	return -1;
}
