#ifndef __CULLING_PROXY__
#define __CULLING_PROXY__


#define MAX_CULLING_CACHE 10

namespace CullingSystem {


class CCullingProxy;

typedef struct CullingCache
{
	CCullingProxy * Proxy;
	int             SubIndex;
}CullingCache;


class CCullingProxy
{
public:
	int m_PoolID;
	int m_Type;
	int m_CacheCount;
	char m_CacheIndex[MAX_CULLING_CACHE];
	CullingCache    m_CullingCache[MAX_CULLING_CACHE];
public:
	CCullingProxy(void);
	virtual ~CCullingProxy(void);
	virtual int Collision(CCullingProxy * Proxy){return 0;};
	virtual int SetCullingCache(CCullingProxy * Proxy, int SubIndex);
	virtual int GetCullingCache(CCullingProxy * Proxy);
};

} // end namespace

#endif
