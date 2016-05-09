#pragma once
#include "Mathlib.h"
#include "common.h"

namespace SceneManager{


class CEntity
{
private:
	int m_RefRenderQueue;
private:
	int m_EntityID;
	int m_MeshID;
	int m_BoneID;
	int m_TextureID;
	bool m_RamOnly;
	//called when resource is create
public:
	void * m_Vertex;
	WORD * m_Index;
	int    m_VSize;
	int    m_INum;
	bool   m_ResourceUsing;
public:
	int GetMeshID();
	CEntity(void);
	~CEntity(void);
	int Create(char * File,char * ModelName, int ModelIndex,int MemoryType);
	int SetGeometryData(void *Vbuffer, int VSize, WORD* IBuffer, int INum);
	int CreateBone(char * File, char * ModelName, int MeshIndex);
	int CreateTexture(wchar_t ** Files);
	int GetBoneID(void);
	int GetTextureID(void);
	int SetEntityID(int ID){m_EntityID = ID;return 0;}
	int GetEntityID(){return m_EntityID;}
	bool IsRamOnly() {return m_RamOnly;}
	Matrix4x4 * GetBoneData(void);
	int GetBoneNum(void);
	int GetFrams(void);
	int Load(h3d_callback callback=NULL, void * Data=NULL);
	int UnLoad(void);
	int RefMesh(void);
	inline int AddQueueRef(void){m_RefRenderQueue++;return 0;};
	inline int DecQueueRef(void){m_RefRenderQueue--;return 0;};
};

}//end namespace
