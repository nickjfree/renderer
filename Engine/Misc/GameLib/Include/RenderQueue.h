#ifndef __RENDER_QUEUE__
#define __RENDER_QUEUE__

#include "ResourceContainer.h"
#include "Entity.h"
#include "Mathlib.h"
#include "SceneNode.h"

using ResourceManager::CResourceContainer;
using SceneManager::CEntity;
using SceneManager::CSceneNode;

// this is the renderqueue

namespace Render{


#define  RENDERABLE_BATCH  64       // renderable num in renderjob
#define  MAX_CMDTYPE       128       // 1 + 16 + 3
#define  MAX_SHADOWMAP      8       // Max Shadowmaps

//Render CMD Types


#define MAX_SHADOW_CMD      4             // max type of commands for shadow rendering
// light pre-pass command,we need 4 of this bacause the vertex-shader are different
#define LPP_TERRAIN         0              
#define LPP_SOLID           1
#define LPP_INSTANCE        2
#define LPP_SKINNING        3

#define LIGHTING            4        // this one is special, see LightProcesser
									 // every ligh need a RenderContext switch,maybe I should find a better way 

#define  HOC                5
#define  SCREEN             6
#define  TERRAIN            7
#define  SOLID              8
#define  INSTANCE           9
#define  SKINNING           10
#define  POST               11
#define  SSLR_TERRAIN       12
#define  GLASS              13
#define  HDR                14

#define  BASIC              17
#define  SPRITE             19
#define  SHADOW             20
#define  SHADOW_INSTANCING  28


#define SHADOW_START       15
#define SHADOW_TERRAIN      0
#define SHADOW_SOLID        1
#define SHADOW_INSTANCE     2
#define SHADOW_SKINNING     3





// object(s) can be draw in one draw call.
// remarks: all the vertex buffers and Index buffers(not always change) are pointers,that means the values can change.
// logic thread changes this values every logic loop,therefor we need a copy of this value to sync between multi-pass
// eg.Light Pre-Pass and Final Geometry Pass. :(
// i use a SceneNode::Refresh(),it update the values to members of it.
typedef struct Renderable
{
	int    IsRenderable;     // valid renderable
	//state
	//short		SkinID;           // Skin create by [Skin] of Render 
	//int			GeometryID;       // Geometry create by [Vertex] of Render
	//int			BoneMatrixID;     // BoneInfo create by [VTF] of Render
	//Matrix4x4 * WorldMatrix;
	//// data segment 
	//DWORD  VSize;
	//void * VertexData;        // when instancing.it is treat as special vertices
	//DWORD  INum;              // index.instancing don;t need these.
	//WORD * IndexData;
	CEntity * Entity;         // entity ,using by render queue, for load/unload,sync
	CSceneNode * Node;        // scenenode used by Render to copy data from logic thread to rendering thread
	                          // the data is stored in scenenode,by calling CSceneNode::Refresh()
	void *  Param;            // additional param
}Renderable;


// RenderJobs use only one same pass,but diffrent textures need to be sort.
typedef struct RenderJob
{
	int         JobID;
	int         NumRenderable;
	RenderJob * NextJob;     // alg
	Renderable  RenderableList[RENDERABLE_BATCH]; 
}RenderJob;

typedef struct RenderCommand
{
	int	   nothing;
	int	   CmdID;
	int    EffectIndex;
	int    TechIndex;
	int    PassIndex;        // better merg these three integer into a 32bit value
	int    ContextKey[CONTEXT_DIM];
	RenderCommand * NextCommand;             // a diff-tree
	RenderCommand * SubCommand;
	RenderJob * CurrentJob;
	RenderJob * Job;
}RenderCommand;

typedef struct RenderState
{
	Matrix4x4 ViewMatrix;      // viewMatrix
	Matrix4x4 Projection;
	Vector3   ViewPoint;       // eye point
	int       ShadowMaps;    // shadow maps
	Matrix4x4 LightMatrix[MAX_SHADOWMAP];     // viewMatrix of light
	Matrix4x4 LightProjection[MAX_SHADOWMAP];
	Vector3   LightPoint[MAX_SHADOWMAP];
}RenderState;

class CRenderQueue
{
private:
	CResourceContainer<RenderCommand>  m_RenderCommandPool;
	CResourceContainer<RenderJob>      m_RenderJobPool;
public:
	RenderState                        m_RenderState;
	RenderCommand *                    m_RenderCMD[MAX_CMDTYPE];         // speed is important
	RenderCommand *                    m_RenderShadowCMD[MAX_SHADOWMAP * MAX_SHADOW_CMD];  // object rendered simply to generate depth, for MAX_SHADOWMAP light sources,normal and instancing
	int                                m_UpToDate;
public:
	CRenderQueue(void);
	~CRenderQueue(void);
	RenderCommand * AllocCommand(void);
	RenderJob * AllocRenderJob(void);
	int FreeCommand(RenderCommand * Command);
	int FreeJob(RenderJob * Job);
	int Reset(void);
	int NewCMD(int Type, int Effect, int Tech, int Pass);
	int AddRenderJob(CEntity * Entity,CMaterial * Material,CSceneNode * Node);
	int AddRenderJob(CEntity * Entity,CMaterial * Material,CSceneNode * Node,int LightIndex);
	int AddRenderJobByContext(int * Context, CEntity * Entity, CSceneNode * Node);
	int AddRenderJobByContext(int * Context, CEntity * Entity, CSceneNode * Node,int LightIndex);
	int AddRenderJobToCommand(RenderCommand * Command,int * Context, CEntity * Entity, CSceneNode * Node);
};

}//end namespace

#endif
