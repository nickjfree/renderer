#ifndef __RENDERRESOURCE__
#define __RENDERRESOURCE__

#include "Mathlib.h"
#include "IMRender.h"
#include "common.h"

#define RES_USING      0x01
#define RES_LOADING    0x02
#define RES_FREE       0x03
#define RES_OUT        0x04
#define RES_DELAY_FREE 0x05


namespace ResourceManager{


typedef struct ResourceDesc
{
	int MemType;
	char * File;
	char * Name;
	int SubIndex;
	// special case, for rawtype texture
	wchar_t ** TextureFileList;
}ResourceDesc;




class CRenderResource
{
public:
	int      m_ResourceID;
	int      m_FreeID;
	int      m_ResourceIDVRAM;
	int      m_ResourceIDRAM;
	int      m_Status;
	int      m_Scale;
	// callback called when resource is create. in main thread
	h3d_callback callback;
	void *  callback_data;
	// free pool list
	CRenderResource * m_Prev, * m_Next;
	char *   m_PackFile;
	bool     m_UnPagedVRAM;
	bool     m_UnPagedRAM;
	bool     m_RAMOnly;
	ResourceDesc  m_Desc;
public:
	CRenderResource(void);
	virtual ~CRenderResource(void);
	virtual int IntoVRAM(void);
	virtual int Load();
	virtual int CreateOnRender(IMRender * Render, int CreateId);
	virtual int Complete();
	int SetPermanent(bool flag);
	virtual int UnLoad(void);
	virtual int ToFreeResource(void);
};


// fixed size 64*64 128*128 256*256  512*512 1024*1024
class CTexture : public CRenderResource
{
public:
	int    m_Scale;
	int    m_FreeIndex;
	int    m_StreamType;
	void * m_RAMDatas[8];
	wchar_t * m_Files[8];
	char * m_StreamFile;
	char * m_Name;
public:
	enum StreamType
	{
		STREAM_NONE,
		STREAM_DN,
		STREAM_HNO,
		STREAM_TYPES,
	};
public:
	CTexture(void);
	~CTexture(void);
	int BindFile(wchar_t ** Filelist);
	int BindStreamFile(char * File, char * Name, int Type);
    virtual int IntoVRAM(void);
	int Init(void);
	virtual int UnLoad();
	virtual int ToFreeResource(void);
	virtual int CreateOnRender(IMRender * Render, int CreateId);
};

class CMesh : public CRenderResource
{
public:
	char        m_File[128];
	char        m_ModelName[32];
	int         m_MeshIndex;
	int			m_VertexType;
	int			m_VSize;
	void *		m_Vertex;
	int			m_IndexNum;
	WORD *		m_Index;
	int         m_Loadable;
	//  size of VRAM
	int         m_VSizeVRAM;
	int         m_IndexNumVRAM;
public:
	CMesh();
	~CMesh();
	int Bind(char * File,char * ModelName,int MeshIndex);
	virtual int SetGeometryData(void * VBuffer, int VSize, WORD* IBuffer, int INum);
	virtual int IntoVRAM(void);
	virtual int UnLoad(void);
	virtual int ToFreeResource(void);
	virtual int CreateOnRender(IMRender * Render, int CreateId);
};

class CBoneInfo : public CRenderResource
{
public:
	char        m_File[128];
	char        m_ModelName[32];
	int         m_MeshIndex;
	int         m_Frames;
	int         m_BoneNum;
	Matrix4x4 * m_BoneData;
public:
	CBoneInfo(void);
	~CBoneInfo(void);
	virtual int IntoVRAM(void);
	int Bind(char * File, char * ModelName, int MeshIndex);
	virtual int Load(void);
};


}//end namespace



#endif
