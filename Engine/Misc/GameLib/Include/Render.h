#ifndef __CRENDER__
#define __CRENDER__

#include "IMRender.h"
#include "RenderQueue.h"
#include "CreationQueue.h"
#include "ResourceContainer.h"
#include "Entity.h"
#include "Material.h"
#include "SceneSoftBody.h"
#include "RenderProcesser.h"
#include "XMLParser.h"
#include "CreationCommand.h"


using namespace Render;
using Render::CProcesser;
using ResourceManager::CResourceContainer;
using SceneManager::CEntity;
using ResourceManager::CreationCommand;
using ResourceManager::CopyCMD;


namespace Render{

class CRenderSettings
{
private:
	float * m_HeightMap;
	int     m_HeightMapWidth;
	int     m_HeightMapHeight;
	int     m_HeitMapFlag;
public:
	IMRender * m_Render;
public:
	CRenderSettings();
	~CRenderSettings();
	void ProcessSettings();
	void SetHeightMap(float * Data, int Width, int Height);
};

class CRender
{
private:
	IMRender *		  m_Render;
	HWND			  m_RenderWindow;
	HANDLE            m_CopyThread;      // copy thread handle
	HANDLE			  m_ThreadHandle;    // rendering thread handle
	CRenderQueue	* m_FrontQueue;      // queue being rendering
	CRenderQueue	* m_BackQueue;       // queue to be rendering
	CRenderQueue    * m_UpdateQueue;     // new queue creating

	CRITICAL_SECTION  m_QueueLock;
	//creation queue
	//CCreationQueue<TextureCMDBatch> m_TexturePermanent;
	//CCreationQueue<MeshCMDBatch>    m_MeshPermanent;
	//CCreationQueue<BoneCMDBatch>    m_BonePermanent;

	//creation queue
	CCreationQueue<CreationCommand>     m_CreationQueue;

	// copy queue
	HANDLE                          m_CopyEvent;
	HANDLE                          m_CopyLock;
	CCreationQueue<CopyCMD>         m_CopyQueue;
	CCreationQueue<CopyCMD>         m_CopyOverQueue;

	// Copy CMD Container
	CResourceContainer<CopyCMD>     m_CopyCMDContainer;
	
	//RenderQueue Processer
	CProcesser *                     m_Processers[MAX_CMDTYPE];
	// width and height
	int                              m_Width;
	int                              m_Height;

	DWORD                            m_LogicThread;

	
private:
	int               m_CurrentCMD;        // Type
	int               m_CurrentSkin;       // SkinID
	int               m_CurrentBoneMatrix; // BoneMatrix
	int               m_CurrentGeometry;   // Static Geometry
	Matrix4x4 *       m_CurrentWorldMatrix;// WorldMatrix

//XML parser
	CXMLParser        m_Parser;
// RenderSettings
	CRenderSettings   m_RenderSettings;
private:
	static CRender * m_ThisRender;
public:
	static CRender * GetRender(void){return m_ThisRender;};

	int GetWidth(void) {return m_Width;};
	int GetHeight(void) {return m_Height;}; 

public:
	CRender(void);
	CRender(DWORD LoagicThread);
	~CRender(void);
	static DWORD  __stdcall RenderThread(LPVOID pParam);
	static DWORD  __stdcall CopyThread(LPVOID pParam);
	int BeginRender(BOOL IsNewThread);
	void CreateRender(HWND hWnd,int width,int height);
	// update one frame to backqueue
	CRenderQueue * UpDateBackQueue(CRenderQueue * BackQueue);
	int SwapQueue(void);
	CRenderQueue * GetUpdateQueue(void);
	int Begin(int Type,int Effect,int Tech,int Pass);
	int SetGeometry(int GeometryID);
	int SetBoneMatrix(int BoneInfoID);
	int AddRenderable(CEntity * Entity,CMaterial * Material,CSceneNode * Node);
	int AddRenderable(CEntity * Entity,CMaterial * Material,CSceneNode * Node, int LightIndex);
	int End(void);
	int SetSkin(int SkinID);
	int ResetUpdateQueue(void);
	int RenderProcess(void);
	int SetViewProjectionMatrix(Matrix4x4& Matrix, Matrix4x4& Projection, Vector3& ViewPoint,Vector3& Look);
	int SetLightViewProjectionMatrix(int LightIndex,Matrix4x4& TransMatrix, Matrix4x4& Projection);
	
	// Add to CreationQueue
	//int CreateTexture(TextureCMDBatch * CMDBatch);
	//int CreateMesh(MeshCMDBatch * Batch);
	//int CreateBone(BoneCMDBatch * Batch);
	int CreateResource(CreationCommand * CMD);


	// Create Resource
	int ProcessResourceCreation(void);
private:
	int AddCopyCMD(CopyCMD * CMD);
	CopyCMD * GetCopyOver(void);
public:
	int AddCopyOver(CopyCMD * CMD);
	CopyCMD * GetCopyCMD(void);
	
private:
	int ProcessCopyCMD(CopyCMD * CMD);
	int ProcessCopyOver(void);
	int ProcessRenderSettings();
public:
	int SetShadowMatrix(int index,float Angle,float Near, float Far,Matrix4x4& ViewMatri,Vector3& Position);
	int InitProcesser(void);

	int ProcessOcclusionQuery(void);

	int InitShader(void);
	int InitRender(HWND hWnd, int Width, int Height); 
	int RenderFrame();
	int SetHeightMap(float * Data,int Width, int Height);
private:
	TechniqueDesc * CreateTechDesc(xml_context ContextTech);
	PassDesc * Render::CRender::CreatePassDesc(xml_context ContextPass);
	DepthStencilDesc * Render::CRender::CreateDepthStencilDesc(xml_context ContextDepthStencil);
	RasterizerDesc * Render::CRender::CreateRasterizerDesc(xml_context ContextRasterizer);
	BlendDesc *  Render::CRender::CreateBlendDesc(xml_context ContextBlend);

};

}//end namespace
#endif
