#ifndef __TEXTURE_LOADER__
#define __TEXTURE_LOADER__


#include "ResourceManager.h"

namespace ResourceManager {

class CTextureLoader : public CResourceLoader
{
private:
	CResourceContainer<CTexture> m_Pool;
	static CTextureLoader * m_ThisLoader;
protected:
	virtual CRenderResource * MatchFreeResource(CRenderResource * Resource);
public:
	CTextureLoader(void);
	virtual ~CTextureLoader(void);
	int LoadTexture(CTexture * Texture);
	virtual CRenderResource *  AllocResource(ResourceDesc * Desc);
	virtual int UseResource(int ID);
	virtual int LoadResource(int ID);
	virtual int UnLoadResource(int ID);
	virtual CRenderResource * GetResource(int ID);
	int UnLoadTexture(CTexture * Texture);
	static CTextureLoader * GetTextureLoader() {return m_ThisLoader;};
};


} //end namespace


#endif