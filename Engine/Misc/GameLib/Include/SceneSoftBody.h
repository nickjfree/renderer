#ifndef   __SOFT_BODY__
#define  __SOFT_BODY__


#include "SceneNode.h"
#include "vertextype.h"


namespace SceneManager{



typedef struct _SoftBodyData
{
	Matrix4x4 Translation;
	vertex_dynamic_instancing * init_vertex;
	vertex_dynamic_instancing * trans_vertex;       // vertics to store transformed values, create by NEW
	int                         VNum;         // vertics number
	WORD *                      index;        // set this value to Entity's index value,the entity should not be free
	int                         INum;         // index number
	Matrix4x4 *                 SkinnkingMatrix; // Bone translations
	Matrix4x4                   SkinningMatrixRender[128];
	int                         Bones;       // BoneNums
}SoftBodyData;



class CSceneSoftBody :public CSceneNode
{

private:
	SoftBodyData m_SoftData;
public:
	CSceneSoftBody(void);
	virtual ~CSceneSoftBody(void);

public:
	virtual void Refresh();
	virtual int Render(int LOD);
	virtual int RenderLightPrePass();
	virtual int RenderShadow(int LOD, int Index);
	// transform the vertex with this matrices,position,normal,and tangent or texcoord
	int SetTransform(Matrix4x4* TransMatrix,int Num);
	SoftBodyData * GetSoftBodyData(){return &m_SoftData;}
	// draw funcions
	virtual int DrawNormalPass(IMRender * Render,int * ContextKey,void * RenderParameter);
	virtual int DrawLightPrePass(IMRender * Render,int * ContextKey,void * RenderParameter);
	virtual int DrawShadowPass(IMRender * Render,int * ContextKey,void * RenderParameter);

}; 

}//end namespace

#endif