#ifndef __MODEL_LOADER__
#define __MODEL_LOADER__

#include "h3d.h"
#include "RenderResource.h"
#include "ResourceManager.h"
#include "FileLoader.h"


#define MAX_PACK_MAPPING  32
#define MAX_ENTRY         10240

using namespace ResourceManager;
using ModelSystem::h3d::h3d_header;
using ModelSystem::h3d::h3d_mesh;
using ModelSystem::h3d::h3d_bone;
using Loader::CFileLoader;

namespace ModelSystem{


typedef struct ModelEntry
{
	char  name[32];
	DWORD offset;
}ModelEntry;


typedef struct ModelFileMapping
{
	int            Ref;
	HANDLE         hFile;
	HANDLE         hMapping;
	void *         Data;
	ModelEntry *   Entry;
	char *         FileName;
}ModelFileMapping;



class CModelLoader : public CFileLoader, public CResourceLoader
{
private:
	static CModelLoader * m_ThisLoader;
	ModelFileMapping      m_ModelMapping[MAX_PACK_MAPPING];
	CResourceContainer<CMesh> m_Pool;
private:
	h3d_mesh   * GetH3DMesh(h3d_header * Header,int MeshIndex);
	h3d_bone * GetH3DBone(h3d_header * header);
public:
	CModelLoader(void);
	~CModelLoader(void);
	int LoadMesh(CMesh * Mesh);
	inline static CModelLoader * GetModelLoader(){return m_ThisLoader;};
	int LoadBone(CBoneInfo * Bone);
	virtual CRenderResource *  AllocResource(ResourceDesc * Desc);
	virtual int UseResource(int ID);
	virtual int LoadResource(int ID);
	virtual int UnLoadResource(int ID);
	virtual CRenderResource * GetResource(int ID);
};

} //end namespace
#endif
