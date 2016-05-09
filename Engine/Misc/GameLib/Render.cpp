#include "StdAfx.h"
#include "Gamelib.h"
#include "Include\Render.h"
#include "RenderQueue.h"
#include "CreationQueue.h"
#include "ResourceManager.h"


#include "LightProcesser.h"
#include "NormalProcesser.h"
#include "LightPreProcesser.h"
#include "PostProcesser.h"
#include "HDRProcesser.h"
#include "ShadowNormalProcesser.h"

using Render::CRender;
using Render::CRenderQueue;
using Render::Renderable;
using Render::RenderCommand;


CRender * CRender::m_ThisRender = NULL;

CRender::CRender(void)
{
	m_BackQueue = new CRenderQueue();
	m_FrontQueue = new CRenderQueue();
	m_UpdateQueue = new CRenderQueue();
	m_ThisRender = this;
	for(int i = 0;i < MAX_CMDTYPE;i++)
	{
		m_Processers[i] = 0;
	}
}

CRender::CRender(DWORD LogicThread)
{
	m_BackQueue = new CRenderQueue();
	m_FrontQueue = new CRenderQueue();
	m_UpdateQueue = new CRenderQueue();
	m_ThisRender = this;
	m_LogicThread = LogicThread;
	for(int i = 0;i < MAX_CMDTYPE;i++)
	{
		m_Processers[i] = 0;
	}
}

CRender::~CRender(void)
{
	DeleteCriticalSection(&m_QueueLock);
}

DWORD  __stdcall Render::CRender::RenderThread(LPVOID pParam)
{
	CRender * Render = (CRender*)pParam;
	//create the render in current thread
	Render->InitRender(NULL,1920,1080);
	
	//main rendering loop

	// shall we enable MRT
	MSG msg;
	while(1)
	{

		if(PeekMessage(&msg,0,0,0,PM_REMOVE))
		{
			if(msg.message == WM_QUIT || msg.message == WM_CLOSE)
			{
				ExitProcess(0);
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			PostThreadMessage(Render->m_LogicThread,msg.message,msg.wParam,msg.lParam);
		}
		else
		{
			Render->RenderFrame();
		}
	}
	return 0;
}

int Render::CRender::BeginRender(BOOL IsNewThread)
{
	// init Render
	InitializeCriticalSection(&m_QueueLock);
	// if create new thread, we create a new thread to handle the rendering loop
	if(IsNewThread)
	{
		m_ThreadHandle = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)RenderThread,this,0,NULL);
	}
	m_CopyThread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)CopyThread,this,0,NULL);
	m_CopyEvent = CreateEvent(NULL,true,false,NULL);
	m_CopyLock  = CreateMutex(NULL,false,NULL);
	return 0;
}

void Render::CRender::CreateRender(HWND hWnd,int width,int height)
{
	m_Render = ::CreateMoonRender();   // call global function
	m_RenderSettings.m_Render = m_Render;
	m_Width = width;
	m_Height = height;
	m_RenderWindow = m_Render->Init(hWnd,width,height,0);
    ShowWindow(m_RenderWindow,SW_SHOW);
    UpdateWindow(m_RenderWindow);
}

// update one fram to backqueue
CRenderQueue * Render::CRender::UpDateBackQueue(CRenderQueue * BackQueue)
{
	CRenderQueue * OldBackQueue;
	EnterCriticalSection(&m_QueueLock);
	OldBackQueue = m_BackQueue;
	m_BackQueue = BackQueue;
	m_BackQueue->m_UpToDate = 1;
	LeaveCriticalSection(&m_QueueLock);
	m_UpdateQueue = OldBackQueue;
	return OldBackQueue;
}

int Render::CRender::SwapQueue(void)
{
	CRenderQueue * NewBackQueue = NULL;
	EnterCriticalSection(&m_QueueLock);
	if(m_BackQueue->m_UpToDate)
	{
		NewBackQueue = m_BackQueue;
		m_BackQueue = m_FrontQueue;
		m_BackQueue->m_UpToDate = 0;
		m_FrontQueue = NewBackQueue;
	}
	LeaveCriticalSection(&m_QueueLock);
	return 0;
}

CRenderQueue * Render::CRender::GetUpdateQueue(void)
{
	return m_UpdateQueue;
}

int Render::CRender::Begin(int Type,int Effect,int Tech,int Pass)
{
	//m_UpdateQueue->NewCMD(Type,Effect,Tech,Pass);
	m_CurrentCMD = Type;
	return 0;
}

int Render::CRender::SetGeometry(int GeometryID)
{
	m_CurrentGeometry = GeometryID;
	return 0;
}

int Render::CRender::SetBoneMatrix(int BoneInfoID)
{
	m_CurrentBoneMatrix = BoneInfoID;
	return 0;
}

int Render::CRender::AddRenderable(CEntity * Entity,CMaterial * Material,CSceneNode * Node)
{
	if(Entity)
	{
		Entity->AddQueueRef();
	}
	m_UpdateQueue->AddRenderJob(Entity,Material,Node);
	return 0;
}

int Render::CRender::AddRenderable(CEntity * Entity,CMaterial * Material,CSceneNode * Node,int LightIndex)
{
	if(Entity)
	{
		Entity->AddQueueRef();
	}
	m_UpdateQueue->AddRenderJob(Entity,Material,Node,LightIndex);
	return 0;
}


int Render::CRender::End(void)
{
	m_CurrentCMD = -1;
	m_CurrentSkin = -1;
	m_CurrentGeometry = -1;
	m_CurrentBoneMatrix = -1;
	m_CurrentWorldMatrix = 0;
	return 0;
}

int Render::CRender::SetSkin(int SkinID)
{
	m_CurrentSkin = SkinID;
	return 0;
}

int Render::CRender::ResetUpdateQueue(void)
{
	m_UpdateQueue->Reset();
	return 0;
}

int Render::CRender::RenderProcess(void)
{
	RenderCommand * Command;
	Renderable * pRenderable = 0;
	RenderState * State;
	//set view
	State = &m_FrontQueue->m_RenderState;

	// shadow pass

	for(int i = 0;i < MAX_SHADOWMAP * MAX_SHADOW_CMD;i++)
	{
		int Slot = i + SHADOW;
		if(m_FrontQueue->m_RenderCMD[Slot])
		{
			//set pass
			Command = m_FrontQueue->m_RenderCMD[Slot];
			if(m_Processers[Slot])
			{
				m_Processers[Slot]->SetRenderState(State);
				m_Processers[Slot]->ProcessCommand(State,Command);
			}
		}
	}

	//light pre-pass and normal pass
	for(int i = LPP_TERRAIN;i < SPRITE+1;i++)
	{
		if(m_FrontQueue->m_RenderCMD[i])
		{
			//set pass
			Command = m_FrontQueue->m_RenderCMD[i];
			if(m_Processers[i])
			{
				m_Processers[i]->SetRenderState(State);
				m_Processers[i]->ProcessCommand(State,Command);
			}
		}
	}
	return 0;
}

int Render::CRender::SetViewProjectionMatrix(Matrix4x4& Matrix,Matrix4x4& Projection, Vector3& ViewPoint,Vector3& Look)
{
	m_UpdateQueue->m_RenderState.ViewMatrix = Matrix;
	m_UpdateQueue->m_RenderState.Projection = Projection;
	m_UpdateQueue->m_RenderState.ViewPoint = ViewPoint;
	return 0;
}



int Render::CRender::SetLightViewProjectionMatrix(int LightIndex, Matrix4x4& View, Matrix4x4& Projection)
{
	m_UpdateQueue->m_RenderState.LightMatrix[LightIndex] = View;
	m_UpdateQueue->m_RenderState.LightProjection[LightIndex] = Projection;
	return 0;
}

int Render::CRender::CreateResource(CreationCommand * CMD)
{
	m_CreationQueue.AddCreationBatch(CMD);
	return 0;
}

int Render::CRender::ProcessResourceCreation(void)
{
	CreationCommand * CMD = NULL;
	int Complete = 0;
	while (CMD = m_CreationQueue.RemoveBatch())
	{
		CMD->Create(m_Render);
		CMD->Complete();
		Complete++;
		// max resource creation per frame
		//if (Complete > MAX_RESOURCE_FRAME)
		//	break;
	}
	return 0;
}
//
//int Render::CRender::ProcessCreation(void)
//{
	//TextureCMDBatch * Batch = m_TexturePermanent.m_BatchHead;
	//CResourceManager * Resource = CResourceManager::GetResourceManager();
	//TextureCMD * CMD;
	//CopyCMDBatch * CopyBatch;
	//while(Batch = m_TexturePermanent.RemoveBatch())
	//{
	//	int CopyID = m_CopyCMDContainer.AllocResource(&CopyBatch);
	//	CopyBatch->ID = CopyID;
	//	CopyBatch->TextureBacth = Batch;
	//	CopyBatch->Type = COPY_TEXTURE;
	//	for(int i = 0;i < CMDBATCH;i++)
	//	{
	//		if(Batch->CMD[i].Id != -1)
	//		{// create it
	//			CMD = &Batch->CMD[i];
	//			if(CMD->CreateId == -1)
	//			{
	//				CMD->CreateId = m_Render->CreateTexture(CMD->TextureName,CMD->FileInMemorey,CMD->Scale,CMD->StreamType);
	//			}
	//			if(CMD->StreamType)
	//			{
	//				/*m_Render->GetDynamicTBuffer(CMD->CreateId, CopyBatch->CopyCmd[i].TBufferDes, CopyBatch->CopyCmd[i].RowPitch);*/
	//				m_Render->GetTextureHandle(CMD->CreateId, CopyBatch->CopyCmd[i].TBufferDes);
	//				CopyBatch->CopyCmd[i].ID = CMD->Id;
	//				CopyBatch->CopyCmd[i].ShaderResourceID = CMD->CreateId;
	//				CopyBatch->CopyCmd[i].Scale = CMD->Scale;
	//				CopyBatch->CopyCmd[i].Type = COPY_TEXTURE;
	//				memcpy(CopyBatch->CopyCmd[i].TBufferRes, CMD->FileInMemorey,8 * sizeof(char*));
	//				if(CMD->Scale)
	//				{
	//					m_Render->UpdateTexture(CopyBatch->CopyCmd[i].TBufferDes,  CMD->FileInMemorey, CMD->Scale, CMD->StreamType);
	//				}
	//			}
	//			else
	//			{
	//				CopyBatch->CopyCmd[i].ID = -1;
	//			}
	//		}
	//		else
	//		{
	//			CopyBatch->CopyCmd[i].ID = -1;
	//		}
	//	}
	//	//the creation is complete now,tell resource manager
	//	AddCopyCMD(CopyBatch);
	//	//Resource->CompleteTexture(Batch);	
//	//}
//	return 0;
//}

//int Render::CRender::CreateMesh(MeshCMDBatch * Batch)
//{
//	m_MeshPermanent.AddCreationBatch(Batch);
//	return 0;
//}

//int Render::CRender::ProcessMesh(void)
//{
//	MeshCMDBatch * Batch;
//	MeshCMD * CMD;
//	CopyCMDBatch * CopyBatch;
//	int CopyID;
//	CResourceManager * Resource = CResourceManager::GetResourceManager();
//	while(Batch = m_MeshPermanent.RemoveBatch())
//	{
//		CopyID = m_CopyCMDContainer.AllocResource(&CopyBatch);
//		CopyBatch->ID = CopyID;
//		CopyBatch->MeshBatch = Batch;
//		CopyBatch->Type = COPY_BUFFER;
//		for(int i = 0;i < CMDBATCH;i++)
//		{
//			if(Batch->CMD[i].Id != -1)
//			{// create it
//				CMD = &Batch->CMD[i];
//				if(CMD->CreateId == -1)
//				{
//					CMD->CreateId = m_Render->CreateDynamicGeometry(CMD->VBuffer,CMD->VSize,CMD->IBuffer,CMD->INum);
//				}
//				m_Render->ChangeDynamicBufferSize(CMD->CreateId,CMD->VSize,CMD->INum);
//				CopyBatch->CopyCmd[i].ID = CMD->Id;
//				CopyBatch->CopyCmd[i].Type = COPY_BUFFER;
//				//CopyBatch->CopyCmd[i].VBufferDes = m_Render->GetDynamicVBuffer(CMD->CreateId);
//				//CopyBatch->CopyCmd[i].IBufferDes = m_Render->GetDynamicIBuffer(CMD->CreateId);
//				CopyBatch->CopyCmd[i].VBufferRes = CMD->VBuffer;
//				CopyBatch->CopyCmd[i].IBufferRes = CMD->IBuffer;
//				CopyBatch->CopyCmd[i].VSize = CMD->VSize;
//				CopyBatch->CopyCmd[i].INum =  CMD->INum;
//				CopyBatch->CopyCmd[i].ShaderResourceID = CMD->CreateId;
//				CopyBatch->CopyCmd[i].GeometryHandle = m_Render->GetDynamicGeometryHandle(CMD->CreateId);
//				m_Render->UpdateDynamicGemometry(CopyBatch->CopyCmd[i].GeometryHandle, CMD->VBuffer, CMD->VSize, CMD->IBuffer, CMD->INum);
//			}
//			else
//			{
//				CopyBatch->CopyCmd[i].ID = -1;
//			}
//		}
//		// Add to copy thread
//		AddCopyCMD(CopyBatch);
//		//the creation is complete now,tell resource manager
////		Resource->CompleteMesh(Batch);	
//	}
//	return 0;
//}

//int Render::CRender::CreateBone(BoneCMDBatch * Batch)
//{
//	m_BonePermanent.AddCreationBatch(Batch);
//	return 0;
//}

//int Render::CRender::ProcessBone(void)
//{
//	BoneCMDBatch * Batch;
//	BoneCMD * CMD;
//	CResourceManager * Resource = CResourceManager::GetResourceManager();
//	while(Batch = m_BonePermanent.RemoveBatch())
//	{
//		for(int i = 0;i < CMDBATCH;i++)
//		{
//			if(Batch->CMD[i].Id != -1)
//			{// create it
//				CMD = &Batch->CMD[i];
//				CMD->CreateId = m_Render->CreateBoneData(CMD->Frames,CMD->BoneData,CMD->BoneNum);
//			}
//		}
//		//the creation is complete now,tell resource manager
//		Resource->CompleteBone(Batch);
//	}
//	return 0;
//}



DWORD  __stdcall Render::CRender::CopyThread(LPVOID pParam)
{
	// IO and Buffer Mapping thread
	CRender * Render = (CRender*)pParam;
	while(1)
	{
		CopyCMD * CMD;
		CMD = Render->GetCopyCMD();
		if(CMD)
		{
			/*for(int i = 0;i < CMDBATCH;i++)
			{
				if(CMD->CopyCmd[i].ID != -1)
				{
					Render->ProcessCopyCMD(&CMD->CopyCmd[i]);
				}
			}*/
			Render->AddCopyOver(CMD);
		}
	}
	return 0;
}

int Render::CRender::AddCopyCMD(CopyCMD * CMD)
{
	WaitForSingleObject(m_CopyLock,-1);
	if(m_CopyQueue.IsEmpty())
	{
		SetEvent(m_CopyEvent);
	}
	m_CopyQueue.AddCreationBatch(CMD);
	ReleaseMutex(m_CopyLock);
	return 0;
}

int Render::CRender::AddCopyOver(CopyCMD * CMD)
{
	m_CopyOverQueue.AddCreationBatch(CMD);
	return 0;
}

CopyCMD * Render::CRender::GetCopyCMD(void)
{
	WaitForSingleObject(m_CopyEvent,-1);
	WaitForSingleObject(m_CopyLock,-1);
	CopyCMD * CMD = m_CopyQueue.RemoveBatch();
	if(!CMD)
	{
		ResetEvent(m_CopyEvent);
	}
	ReleaseMutex(m_CopyLock);
	return CMD;
}

CopyCMD * Render::CRender::GetCopyOver(void)
{
	return m_CopyOverQueue.RemoveBatch();
}

int Render::CRender::ProcessCopyCMD(CopyCMD * CMD)
{
	if(CMD->Type == COPY_BUFFER)
	{
		//// copy vertex
		//memcpy(CMD->VBufferDes,CMD->VBufferRes,CMD->VSize);
		//// copy index
		//memcpy(CMD->IBufferDes,CMD->IBufferRes,CMD->INum * sizeof(WORD));
		//m_Render->UpdateDynamicGemometry(CMD->GeometryHandle, CMD->VBufferRes, CMD->VSize, CMD->IBufferRes, CMD->INum);
	}
	if(CMD->Type == COPY_TEXTURE)
	{
		/*if(CMD->Scale)
		{
			m_Render->UpdateTexture(CMD->TBufferDes, CMD->TBufferRes, CMD->Scale, CMD->Type);
		}*/
		/*for(int i = 0; i < 8; i++)
		{
			if(CMD->TBufferDes[i])
			{
				int Scale = CMD->Scale;
				for(int h = 0; h < Scale; h++)
				{
					int Size = Scale * sizeof(float);
					void * RowStart = (char*)CMD->TBufferDes[i] + CMD->RowPitch[i] * h;
					void * ResStart = (char*)CMD->TBufferRes[i] + Size * h;
					memcpy(RowStart, ResStart, Size);
				}
			}
		}*/
		
	}
	return 0;
}

int Render::CRender::ProcessCopyOver(void)
{
	CopyCMD * CMD = 0;
	while(CMD = GetCopyOver())
	{
		for(int i = 0;i < CMDBATCH;i++)
		{
//			if(CMD->CopyCmd[i].ID != -1)
//			{
//				if(CMD->Type == COPY_BUFFER)
//				{
////					m_Render->PutDyanmicBuffer(CMD->CopyCmd[i].ShaderResourceID);
//				}
//				if(CMD->Type == COPY_TEXTURE)
//				{	
////					m_Render->PutDynamicTBuffer(CMD->CopyCmd[i].ShaderResourceID);
//				}
//				if(CMD->Type == COPY_BONE)
//				{
//
//				}
//			}
		}
		/*if(CMD->Type == COPY_BUFFER)
		{
			Resource->CompleteMesh(CMD->MeshBatch);
		}
		if(CMD->Type == COPY_TEXTURE)
		{
			Resource->CompleteTexture(CMD->TextureBacth);
		}
		if(CMD->Type == COPY_BONE)
		{
			Resource->CompleteBone(CMD->BoneBatch);
		}*/
		m_CopyCMDContainer.FreeResource(CMD->ID);
	}
	return 0;
}

int Render::CRender::ProcessRenderSettings()
{
	m_RenderSettings.ProcessSettings();
	return 0;
}

int Render::CRender::ProcessOcclusionQuery()
{
	m_Render->CheckOcclusionQuery();
	return 0;
}

int Render::CRender::SetShadowMatrix(int Index,float Angel,float Near, float Far,Matrix4x4& ViewMatrix,Vector3& Position)
{
	if(m_UpdateQueue->m_RenderState.ShadowMaps < Index+1)
	{
		m_UpdateQueue->m_RenderState.ShadowMaps = Index+1;
	}
	m_UpdateQueue->m_RenderState.LightMatrix[Index] = ViewMatrix;
	m_UpdateQueue->m_RenderState.LightPoint[Index] = Position;
	//m_UpdateQueue->m_RenderState.ViewMatrix = ViewMatrix;
	return 0;
}

int Render::CRender::InitProcesser(void)
{
	//terrain
	CNormalProcesser * NormalProcesser = new CNormalProcesser(m_Render);
	CLightPreProcesser * LightPreProcesser = new CLightPreProcesser(m_Render);
	CLightProcesser * LightProcesser = new CLightProcesser(m_Render);
	CPostProcesser * PostProcesser = new CPostProcesser(m_Render);
	CHDRProcesser * HDRProcesser = new CHDRProcesser(m_Render);

	m_Processers[TERRAIN] = NormalProcesser;
	//solid
	m_Processers[SOLID] = NormalProcesser;
	//instancing
	m_Processers[INSTANCE] = NormalProcesser;
	//// skinning
	m_Processers[SKINNING] = NormalProcesser;
	////basic geometry
	//m_Processers[BASIC] = NormalProcesser;
	//// lighting
	m_Processers[LIGHTING] = LightProcesser;
	////sprite

	//post
	m_Processers[POST] = PostProcesser;
	m_Processers[SSLR_TERRAIN] = PostProcesser;
	m_Processers[GLASS] = PostProcesser;

	//HDR
	m_Processers[HDR] = HDRProcesser;
	//// Light Pre-Pass processers
	m_Processers[LPP_TERRAIN] = LightPreProcesser;
	m_Processers[LPP_SOLID]   = LightPreProcesser;
	m_Processers[LPP_INSTANCE] = LightPreProcesser;
	m_Processers[LPP_SKINNING]  = LightPreProcesser;
	//// Hardware Occlusion Query
	m_Processers[HOC] = NormalProcesser;

	// set processers for shadow cmd
	for(int i = 0;i < MAX_SHADOWMAP;i++)
	{
		CShadowNormalProcesser * ShadowNormalProcesser = new CShadowNormalProcesser(m_Render);
		ShadowNormalProcesser->SetLightIndex(i);
		m_Processers[SHADOW + i * MAX_SHADOW_CMD + SHADOW_TERRAIN] = ShadowNormalProcesser;
		m_Processers[SHADOW + i * MAX_SHADOW_CMD + SHADOW_SOLID] = ShadowNormalProcesser;
		m_Processers[SHADOW + i * MAX_SHADOW_CMD + SHADOW_INSTANCE] = ShadowNormalProcesser;
		m_Processers[SHADOW + i * MAX_SHADOW_CMD + SHADOW_SKINNING] = ShadowNormalProcesser;
	}
	return 0;
}

TechniqueDesc * Render::CRender::CreateTechDesc(xml_context ContextTech)
{
	PassDesc * PDesc,* FirstPass = 0,* LastPass = 0;
	TechniqueDesc * TechDesc = new TechniqueDesc();
	TechDesc->name = m_Parser.GetAttribString(ContextTech,"","name");
	xml_context context_pass = m_Parser.GetFirstContext(ContextTech,"pass");
	while(context_pass)
	{
		PDesc = CreatePassDesc(context_pass);
		if(!FirstPass)
		{
			FirstPass = LastPass = PDesc;
		}
		else
		{
			LastPass->next = PDesc;
			LastPass = PDesc;
		}
		TechDesc->pass = FirstPass;
		context_pass = m_Parser.GetNextContext(context_pass);
	}
	return TechDesc;
}

PassDesc * Render::CRender::CreatePassDesc(xml_context ContextPass)
{
	PassDesc * PDesc = new PassDesc();
	PDesc->name = m_Parser.GetAttribString(ContextPass,"","name");
	PDesc->next = 0;
	PDesc->VTypeId = m_Parser.GetAttribInt(ContextPass,"","vtypeid");
	PDesc->VSFile = PDesc->PSFile = m_Parser.GetAttribString(ContextPass,"vs","file");
	PDesc->VSMain = m_Parser.GetAttribString(ContextPass,"vs","entry");
	PDesc->PSMain = m_Parser.GetAttribString(ContextPass,"ps","entry");
	PDesc->DepthId = m_Parser.GetAttribInt(ContextPass,"","depthstencil_index");
	PDesc->ResterizerId = m_Parser.GetAttribInt(ContextPass,"","rasterizer_index");
	PDesc->BlendId = m_Parser.GetAttribInt(ContextPass,"","blend_index");
	PDesc->Primitive = m_Parser.GetAttribInt(ContextPass, "", "primitive");
	if(PDesc->BlendId)
	{
		int a = 0;
		a++;
	}
	return PDesc;
}

DepthStencilDesc * Render::CRender::CreateDepthStencilDesc(xml_context ContextDepthStencil)
{
	DepthStencilDesc * DSDesc = new DepthStencilDesc();
	DSDesc->DepthEnable = m_Parser.GetAttribInt(ContextDepthStencil,"DepthEnable",NULL);
	DSDesc->DepthFunc = m_Parser.GetAttribString(ContextDepthStencil,"DepthFunc",NULL);
	DSDesc->StencilEnable = m_Parser.GetAttribInt(ContextDepthStencil,"StencilEnable",NULL);
	DSDesc->DepthWriteMask = m_Parser.GetAttribInt(ContextDepthStencil,"DepthWriteMask",NULL);

	DSDesc->FrontStencilFailOp = m_Parser.GetAttribString(ContextDepthStencil,"FrontStencilFailOp",NULL);
	DSDesc->FrontStencilDepthFailOp = m_Parser.GetAttribString(ContextDepthStencil,"FrontStencilDepthFailOp",NULL);
	DSDesc->FrontStencilPassOp = m_Parser.GetAttribString(ContextDepthStencil,"FrontStencilPassOp",NULL);
	DSDesc->FrontStencilFunc = m_Parser.GetAttribString(ContextDepthStencil,"FrontStencilFunc",NULL);

	DSDesc->BackStencilFailOp = m_Parser.GetAttribString(ContextDepthStencil,"BackStencilFailOp",NULL);
	DSDesc->BackStencilDepthFailOp = m_Parser.GetAttribString(ContextDepthStencil,"BackStencilDepthFailOp",NULL);
	DSDesc->BackStencilPassOp = m_Parser.GetAttribString(ContextDepthStencil,"BackStencilPassOp",NULL);
	DSDesc->BackStencilFunc = m_Parser.GetAttribString(ContextDepthStencil,"BackStencilFunc",NULL);

	DSDesc->StencilRef = m_Parser.GetAttribInt(ContextDepthStencil,"","ref");
	// write mask is always -1
	DSDesc->StencilWriteMask = -1;
	DSDesc->StencilReadMask = -1;
	return DSDesc;
}

RasterizerDesc * Render::CRender::CreateRasterizerDesc(xml_context ContextRasterizer)
{
	RasterizeDesc * RSDesc = new RasterizeDesc();
	RSDesc->FillMode = m_Parser.GetAttribString(ContextRasterizer,"FillMode",NULL);
	RSDesc->CullMode = m_Parser.GetAttribString(ContextRasterizer,"CullMode",NULL);
	return RSDesc;
}

BlendDesc *  Render::CRender::CreateBlendDesc(xml_context ContextBlend)
{
	BlendDesc * BDesc = new BlendDesc();
	BDesc->BlendEnable = m_Parser.GetAttribInt(ContextBlend,"BlendEnable",NULL);
	BDesc->RenderTargetWriteMask = m_Parser.GetAttribString(ContextBlend,"RenderTargetWriteMask",NULL);
	BDesc->SrcBlend = m_Parser.GetAttribString(ContextBlend,"SrcBlend",NULL);
	BDesc->DestBlend = m_Parser.GetAttribString(ContextBlend,"DestBlend",NULL);
	BDesc->BlendOp = m_Parser.GetAttribString(ContextBlend,"BlendOp",NULL);
	BDesc->SrcBlendAlpha = m_Parser.GetAttribString(ContextBlend,"SrcBlendAlpha",NULL);
	BDesc->DestBlendAlpha = m_Parser.GetAttribString(ContextBlend,"DestBlendAlpha",NULL);
	BDesc->BlendOpAlpha = m_Parser.GetAttribString(ContextBlend,"BlendOpAlpha",NULL);
	return BDesc;
}

int Render::CRender::InitShader()
{
	//this is test(the real one will in script file)
	// now it is xml file instead,script is not good
	TechniqueDesc * TechDesc,* FirstTech = 0,* LastTech = 0;
	RasterizerDesc * RSDesc,* FirstRS = NULL,* LastRS;
	DepthStencilDesc * DSDesc,*FirstDS = NULL,*LastDS;
	BlendDesc * BDesc,*FirstB=NULL,*LastB;
	//load xml file and pars it to PassDesc,TechDesc
	m_Parser.Parse("shaders\\shaders.xml");
	xml_context context_tech = m_Parser.GetFirstContext(NULL,"technique");
	while(context_tech)
	{
		
		// set up test
		TechDesc = CreateTechDesc(context_tech);
		if(!FirstTech)
		{
			FirstTech = LastTech = TechDesc;
		}
		else
		{
			LastTech->next = TechDesc;
			LastTech = TechDesc;
		}
		TechDesc->next = 0;
		context_tech = m_Parser.GetNextContext(context_tech);
	}
	// parse depthstencil desc
	xml_context context_depth = m_Parser.GetFirstContext(NULL,"DepthStencil");
	while(context_depth)
	{
		
		// set up test
		DSDesc = CreateDepthStencilDesc(context_depth);
		if(!FirstDS)
		{
			FirstDS = LastDS = DSDesc;
		}
		else
		{
			LastDS->next = DSDesc;
			LastDS = DSDesc;
		}
		DSDesc->next = 0;
		context_depth = m_Parser.GetNextContext(context_depth);
	}
	// parse rasterizer desc
	xml_context context_rasterizer = m_Parser.GetFirstContext(NULL,"Rasterizer");
	while(context_rasterizer)
	{
		
		// set up test
		RSDesc = CreateRasterizerDesc(context_rasterizer);
		if(!FirstRS)
		{
			FirstRS = LastRS = RSDesc;
		}
		else
		{
			LastRS->next = RSDesc;
			LastRS = RSDesc;
		}
		RSDesc->next = 0;
		context_rasterizer = m_Parser.GetNextContext(context_rasterizer);
	}
	// and blend desc,i think this is the end
	xml_context context_blend = m_Parser.GetFirstContext(NULL,"Blend");
	while(context_blend)
	{
		
		// set up test
		BDesc = CreateBlendDesc(context_blend);
		if(!FirstB)
		{
			FirstB = LastB =BDesc;
		}
		else
		{
			LastB->next = BDesc;
			LastB = BDesc;
		}
		BDesc->next = 0;
		context_blend = m_Parser.GetNextContext(context_blend);
	}
	m_Render->InitShader(FirstTech,FirstDS,FirstRS,FirstB);
	return 0;
}


int CRender::InitRender(HWND hWnd, int Width, int Height)
{
	CreateRender(hWnd,Width,Height);
	InitProcesser();
	InitShader();
	return 0;
}

int CRender::RenderFrame()
{
//	Sleep(10);
	//resource creation here
	ProcessResourceCreation();
	ProcessCopyOver();
	//read query result back
	ProcessOcclusionQuery();
	//preprocess 
	//Clear test
	SwapQueue();
	m_Render->BeginRender(0x000000B0);
	//
	ProcessRenderSettings();
	//we do real rendering here geometry pass and light pass
	RenderProcess();

	// post pass,SSAO,deffered rendering
	//MoonRender->EnablePostPass();
	//MoonRender->PostProcess();                // direction light
	m_Render->EndRender();
	return 0;
}



CRenderSettings::CRenderSettings():m_HeitMapFlag(0),m_Render(0)
{
}

CRenderSettings::~CRenderSettings()
{
}
	
void CRenderSettings::ProcessSettings()
{
	if(!m_Render)
	{
		return;
	}
}